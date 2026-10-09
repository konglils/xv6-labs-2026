#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int ticks;

  if (argc != 2) {
    fprintf(2, "Usage: sleep ticks\n");
  }
  ticks = atoi(argv[1]);

  if (pause(ticks) < 0) {
    fprintf(2, "sleep failed\n");
  }

  exit(0);
}
