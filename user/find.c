#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "user/user.h"

static char **cmd; // -exec command and its arguments, or 0 to print
static int ncmd;

static const char *
basename(const char *path)
{
  const char *p, *base = path;

  for (p = path; *p; p++)
    if (*p == '/' && p[1] != '\0')
      base = p + 1;
  return base;
}

static int
run(char *path)
{
  char *argv[MAXARG];
  int i, pid, status;

  if (cmd == 0) {
    printf("%s\n", path);
    return 0;
  }

  for (i = 0; i < ncmd; i++)
    argv[i] = cmd[i];
  argv[ncmd] = path;
  argv[ncmd + 1] = 0;

  if ((pid = fork()) < 0) {
    fprintf(2, "find: fork failed\n");
    return -1;
  }
  if (pid == 0) {
    exec(argv[0], argv);
    fprintf(2, "find: exec %s failed\n", argv[0]);
    exit(1);
  }
  if (wait(&status) < 0) {
    fprintf(2, "find: wait failed\n");
    return -1;
  }
  return 0;
}

static int
find(char *path, const char *name)
{
  char buf[512], *p;
  struct dirent de;
  struct stat st;
  int fd, n, err = 0;

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return -1;
  }
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return -1;
  }

  if (strcmp(basename(path), name) == 0 && run(path) < 0)
    err = -1;

  if (st.type != T_DIR) {
    close(fd);
    return err;
  }

  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
    fprintf(2, "find: path too long: %s\n", path);
    close(fd);
    return -1;
  }
  strcpy(buf, path);
  p = buf + strlen(buf);
  if (p[-1] != '/')
    *p++ = '/';

  while ((n = read(fd, &de, sizeof(de))) == sizeof(de)) {
    if (de.inum == 0)
      continue;
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = '\0';
    if (strcmp(p, ".") == 0 || strcmp(p, "..") == 0)
      continue;
    if (find(buf, name) < 0)
      err = -1;
  }
  if (n < 0) {
    fprintf(2, "find: cannot read %s\n", path);
    err = -1;
  }

  close(fd);
  return err;
}

int
main(int argc, char *argv[])
{
  if (argc == 3) {
    cmd = 0;
  } else if (argc >= 5 && strcmp(argv[3], "-exec") == 0) {
    cmd = &argv[4];
    ncmd = argc - 4;
    if (ncmd + 2 > MAXARG) {
      fprintf(2, "find: too many arguments to -exec\n");
      exit(1);
    }
  } else {
    fprintf(2, "Usage: find path name [-exec cmd [args...]]\n");
    exit(1);
  }

  exit(find(argv[1], argv[2]) < 0 ? 1 : 0);
}
