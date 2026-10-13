#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define SEPARATORS " -\r\t\n./,"
#define MAXDIGITS  32

// A token is a number only if it is made entirely of digits and bounded by
// separators (or start/end of input), so "xv6" is not a number but "/6," is.
static void
flush(char *num, int len, int valid)
{
  int i;
  uint value5 = 0, value6 = 0;

  if (!valid || len == 0)
    return;
  // Track remainders instead of the value so arbitrarily long numbers work.
  for (i = 0; i < len; i++) {
    value5 = (value5 * 10 + (num[i] - '0')) % 5;
    value6 = (value6 * 10 + (num[i] - '0')) % 6;
  }
  if (value5 != 0 && value6 != 0)
    return;
  for (i = 0; i < len - 1 && num[i] == '0'; i++)
    ;
  printf("%s\n", num + i);
}

static int
sixfive(int fd)
{
  char c;
  char num[MAXDIGITS + 1];
  int len = 0;
  int valid = 1;
  int n;

  while ((n = read(fd, &c, 1)) == 1) {
    if (c >= '0' && c <= '9') {
      if (len < MAXDIGITS)
        num[len++] = c;
      else
        valid = 0;
    } else if (c != '\0' && strchr(SEPARATORS, c)) {
      num[len] = '\0';
      flush(num, len, valid);
      len = 0;
      valid = 1;
    } else {
      valid = 0;
    }
  }
  num[len] = '\0';
  flush(num, len, valid);

  return n < 0 ? -1 : 0;
}

int
main(int argc, char *argv[])
{
  int i, fd;
  int status = 0;

  if (argc < 2) {
    if (sixfive(0) < 0) {
      fprintf(2, "sixfive: read error\n");
      exit(1);
    }
    exit(0);
  }

  for (i = 1; i < argc; i++) {
    if ((fd = open(argv[i], O_RDONLY)) < 0) {
      fprintf(2, "sixfive: cannot open %s\n", argv[i]);
      status = 1;
      continue;
    }
    if (sixfive(fd) < 0) {
      fprintf(2, "sixfive: read error on %s\n", argv[i]);
      status = 1;
    }
    close(fd);
  }
  exit(status);
}
