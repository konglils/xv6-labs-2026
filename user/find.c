#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

void
find(char *buf, int maxsize, char *filename)
{
  char namebuf[DIRSIZ + 1] = {};
  int fd, dirlen = strlen(buf);
  struct dirent de;
  struct stat st;

  if ((fd = open(buf, O_RDONLY)) < 0) {
    fprintf(2, "find: cannot open %s", buf);
    return;
  }

  if (dirlen + 1 + DIRSIZ + 1 > maxsize) {
    fprintf(2, "find: path too long");
    close(fd);
    return;
  }

  buf[dirlen] = '/';

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    memmove(namebuf, de.name, DIRSIZ);
    if (de.inum == 0
        || strcmp(namebuf, ".") == 0
        || strcmp(namebuf, "..") == 0)
      continue;

    strcpy(buf + dirlen + 1, namebuf);

    if (stat(buf, &st) < 0) {
      fprintf(2, "find: cannot stat %s\n", buf);
      continue;
    }

    if (strcmp(namebuf, filename) == 0)
      printf("%s\n", buf);

    if (st.type == T_DIR)
      find(buf, maxsize, filename);
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  char buf[512];
  struct stat st;

  if (argc != 3) {
    fprintf(2, "Usage: find directory filename\n");
    exit(1);
  }

  if (stat(argv[1], &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", argv[1]);
    exit(1);
  }

  if (st.type != T_DIR) {
    fprintf(2, "find: %s is not directory\n", argv[1]);
    exit(1);
  }

  strcpy(buf, argv[1]);
  find(buf, sizeof(buf), argv[2]);

  exit(0);
}
