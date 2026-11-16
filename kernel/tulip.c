//
// A driver for qemu's "tulip" DEC 21143 NIC.
//
// see: Digital Semiconductor 21143 PCI/CardBus 10/100-Mb/s Ethernet
//      LAN Controller Hardware Reference Manual
//

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "tulip.h"

struct tulip_private tp;

//
// called by pci_init().
// csrs is the memory address at which the
// tulip's 16 command and status registers (CSRs) are mapped.
//
void
tulip_init(volatile uint64 *csrs)
{
  // Your code here.
  char *buf;
  int i;

  for (i = 0; i < RX_RING_SIZE; i++) {
    buf = kalloc();
    if (!buf)
      panic("tulip_init: kalloc failed\n");
    tp.rx_ring[i].buffer1 = (uint64)buf;
    tp.rx_ring[i].buffer2 = 0;
    tp.rx_ring[i].length = 1020;
    tp.rx_ring[i].status = DescOwned;
  }
  tp.rx_ring[i - 1].length |= DescEndRing;

  for (i = 0; i < TX_RING_SIZE; i++) {
    tp.tx_ring[i].status = 0;
    tp.tx_ring[i].buffer2 = 0;
  }

  tp.base_addr = (char *)csrs;

  write_csr(&tp, CSR3, (uint64)tp.rx_ring);
  write_csr(&tp, CSR4, (uint64)tp.tx_ring);
  tp.cur_rx = tp.cur_tx = 0;
  tp.dirty_rx = tp.dirty_tx = 0;

  write_csr(&tp, CSR7, NormalIntr | RxNoBuf | RxIntr | TxNoBuf | TxIntr);
  write_csr(&tp, CSR6, MBO | TxOn | PrOn | RxOn);
}

//
// buf contains an ethernet frame.
// tulip_xmit() should ask the tulip NIC to transmit the packet.
// the tulip driver should kfree(buf) when it's done.
//
void
tulip_xmit(char *buf, int len)
{
  // Your code here.
  int entry;
  uint32 flag;

  acquire(&tp.lock);

  while (tp.cur_tx - tp.dirty_tx >= TX_RING_SIZE) {
    sleep_prepare(&tp.chan);
    release(&tp.lock);
    sleep();
    acquire(&tp.lock);
  }

  entry = tp.cur_tx % TX_RING_SIZE;

  tp.tx_ring[entry].buffer1 = (uint64)buf;

  flag = DescTxLast | DescTxFirst;
  if (entry == TX_RING_SIZE - 1)
    flag |= DescEndRing;
  tp.tx_ring[entry].length = len | flag;
  tp.tx_ring[entry].status = DescOwned;

  tp.cur_tx++;

  write_csr(&tp, CSR1, 1);

  release(&tp.lock);
}

void
tulip_rx()
{
  int entry = tp.cur_rx % RX_RING_SIZE;
  int rx_work_limit = tp.dirty_rx + RX_RING_SIZE - tp.cur_rx;

  while (!(tp.rx_ring[entry].status & DescOwned)) {
    uint32 status = tp.rx_ring[entry].status;

    if (--rx_work_limit < 0)
      break;

    int pkt_len = ((status >> 16) & 0x3fff) - 4;
    char *inbuf = (char *)(uint64)tp.rx_ring[entry].buffer1;
    net_rx(inbuf, pkt_len);

    tp.rx_ring[entry].buffer1 = 0;

    entry = (++tp.cur_rx) % RX_RING_SIZE;
  }
}

void
tulip_refill_rx()
{
  for (; tp.cur_rx - tp.dirty_rx > 0; tp.dirty_rx++) {
    int entry = tp.dirty_rx % RX_RING_SIZE;
    if (tp.rx_ring[entry].buffer1 == 0) {
      char *buf = kalloc();
      if (!buf)
        panic("tulip_refill_rx: kalloc failed");
      tp.rx_ring[entry].buffer1 = (uint64)buf;
    }
    tp.rx_ring[entry].status = DescOwned;
  }

  write_csr(&tp, CSR2, 1);
}

void
tulip_intr(void)
{
  int csr5;

  csr5 = read_csr(&tp, CSR5);

  if ((csr5 & NormalIntr) == 0)
    return;

  do {
    // clear these bits to generate new interrupts;
    write_csr(&tp, CSR5, csr5 & 0x0001ffff);

    if (csr5 & (RxIntr | RxNoBuf)) {
      tulip_rx();
      tulip_refill_rx();
    }

    if (csr5 & (TxIntr | TxNoBuf)) {
      acquire(&tp.lock);

      for (; tp.cur_tx - tp.dirty_tx > 0; tp.dirty_tx++) {
        int entry = tp.dirty_tx % TX_RING_SIZE;

        if (tp.tx_ring[entry].status & DescOwned)
          break;

        kfree((char *)(uint64)tp.tx_ring[entry].buffer1);
        tp.tx_ring[entry].buffer1 = 0;
      }

      wakeup(&tp.chan);

      release(&tp.lock);
    }

    csr5 = read_csr(&tp, CSR5);

  } while ((csr5 & NormalIntr) != 0);

  tulip_refill_rx();
}
