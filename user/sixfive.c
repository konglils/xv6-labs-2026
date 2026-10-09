#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

void
sixfive(int fd)
{
  int n;
  char c;

  n = 0;
  while (read(fd, &c, 1) > 0) {
    if (strchr(" -\r\t\n./,", c)) {
      if (n % 6 == 0 || n % 5 == 0) {
        printf("%d\n", n);
      }
      n = 0;
    } else {
      n = n * 10 + c - '0';
    }
  }
  if (n % 6 == 0 || n % 5 == 0) {
    printf("%d\n", n);
  }
}

int
main(int argc, char *argv[])
{
  int fd, i;

  if (argc <= 1) {
    sixfive(0);
    exit(0);
  }

  for (i = 1; i < argc; i++) {
    fd = open(argv[i], O_RDONLY);
    if (fd < 0) {
      fprintf(2, "cannot open %s\n", argv[i]);
      exit(1);
    }
    sixfive(fd);
    if (close(fd) < 0) {
      fprintf(2, "cannot close %s\n", argv[i]);
      exit(1);
    }
  }

  exit(0);
}
