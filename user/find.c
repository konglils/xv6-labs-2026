#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"

void
runcmd(int argc, char *argv[], char *path)
{
  static char *buf[MAXARG + 1]; // add 1 for null pointer
  int i;

  if (argc + 1 >= MAXARG) {
    fprintf(2, "find: command too long\n");
    return;
  }

  switch (fork()) {
  case -1:
    fprintf(2, "find: fork failed\n");
    break;

  case 0:
    for (i = 0; i < argc; i++) {
      buf[i] = argv[i];
    }
    buf[i] = path;
    buf[i + 1] = 0;

    exec(buf[0], buf);
    // exec will return if it fails
    fprintf(2, "find: exec failed");
    break;

  default:
    wait(0);
  }
}

void
find(char *buf, int maxsize, char *filename, int argc, char *argv[])
{
  char name[DIRSIZ + 1] = {};
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
    memmove(name, de.name, DIRSIZ);
    if (de.inum == 0 || strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      continue;

    strcpy(buf + dirlen + 1, name);

    if (stat(buf, &st) < 0) {
      fprintf(2, "find: cannot stat %s\n", buf);
      continue;
    }

    if (strcmp(name, filename) == 0) {
      if (argc == 0)
        printf("%s\n", buf);
      else
        runcmd(argc, argv, buf);
    }

    if (st.type == T_DIR)
      find(buf, maxsize, filename, argc, argv);
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  char buf[512];
  struct stat st;

  if (argc < 3) {
    fprintf(2, "Usage: find directory filename [-exec command]\n");
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

  if (argc == 3) {
    find(buf, sizeof(buf), argv[2], 0, 0);
  } else if (strcmp(argv[3], "-exec") == 0) {
    if (argc == 4) {
      fprintf(2, "find: missing argument to `-exec'\n");
      exit(1);
    }
    find(buf, sizeof(buf), argv[2], argc - 4, &argv[4]);
  } else {
    fprintf(2, "Usage: find directory filename [-exec command]\n");
    exit(1);
  }

  exit(0);
}
