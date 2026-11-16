enum csr_offsets {
  CSR1 = 0x08,
  CSR2 = 0x10,
  CSR3 = 0x18,
  CSR4 = 0x20,
  CSR5 = 0x28,
  CSR6 = 0x30,
  CSR7 = 0x38,
};

enum status_bits {
  NormalIntr = 0x10000,
  RxNoBuf = 0x80,
  RxIntr = 0x40,
  TxNoBuf = 0x04,
  TxIntr = 0x01,
};

enum mode_bits {
  MBO = 0x2000000,
  TxOn = 0x2000,
  PrOn = 0x40,
  RxOn = 0x02,
};

struct rx_desc {
  uint32 status;
  uint32 length;
  uint32 buffer1;
  uint32 buffer2;
};

struct tx_desc {
  uint32 status;
  uint32 length;
  uint32 buffer1;
  uint32 buffer2;
};

enum desc_bits {
  DescOwned = 0x80000000,

  DescEndRing = 0x02000000,

  DescTxDone = 0x80000000,
  DescTxLast = 0x40000000,
  DescTxFirst = 0x20000000,
};

#define TX_RING_SIZE 16
#define RX_RING_SIZE 16

struct tulip_private {
  struct rx_desc rx_ring[RX_RING_SIZE];
  struct tx_desc tx_ring[TX_RING_SIZE];
  uint cur_rx, cur_tx;     /* The next free ring entry */
  uint dirty_rx, dirty_tx; /* The ring entries to be freeed. */
  struct spinlock lock;
  char *base_addr;
  int chan;
};

static inline uint32
read_csr(struct tulip_private *tp, int csr_offset)
{
  return *(volatile uint32 *)(tp->base_addr + csr_offset);
}

static inline void
write_csr(struct tulip_private *tp, int csr_offset, uint32 val)
{
  *(volatile uint32 *)(tp->base_addr + csr_offset) = val;
}
