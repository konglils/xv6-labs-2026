#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

#define DATASIZE (100 * 4096)

char *
attack(char *data, char *end)
{
  char *msg = "Here it is: ";
  while (*msg && data < end) {
    if (*msg++ != *data++)
      return 0;
  }
  if (*data == 0)
    return 0;
  else
    return data;
}

int
main(int argc, char *argv[])
{
  // Your code here.
  char *data = sbrk(DATASIZE);
  char *end = data + DATASIZE;
  for (; data < end; data++) {
    char *secret = attack(data, end - 1);
    if (secret) {
      printf("%s\n", secret);
      exit(0);
    }
  }

  exit(0);
}
