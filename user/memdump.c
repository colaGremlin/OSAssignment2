#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void memdump(char *fmt, char *data, int len);

int
main(int argc, char *argv[])
{
  if (argc == 1) {
    printf("Example 1:\n");
    int a[2] = {61810, 2026};
    memdump("ii", (char *)a, sizeof(a));

    printf("Example 2:\n");
    memdump("S", "a string", sizeof("a string"));

    printf("Example 3:\n");
    char *s = "another";
    memdump("s", (char *)&s, sizeof(s));

    struct sss {
      char *ptr;
      int num1;
      short num2;
      char byte;
      char bytes[8];
    } example;

    example.ptr = "hello";
    example.num1 = 1819438967;
    example.num2 = 100;
    example.byte = 'z';
    strcpy(example.bytes, "xyzzy");

    printf("Example 4:\n");
    memdump("pihcS", (char *)&example, sizeof(example));

    printf("Example 5:\n");
    memdump("sccccc", (char *)&example, sizeof(example));
  } else if (argc == 2) {
    // format in argv[1], up to 512 bytes of data from standard input.
    char data[512];
    int n = 0;
    memset(data, '\0', sizeof(data));
    while (n < sizeof(data)) {
      int nn = read(0, data + n, sizeof(data) - n);
      if (nn <= 0)
        break;
      n += nn;
    }
    memdump(argv[1], data, n);
  } else {
    printf("Usage: memdump [format]\n");
    exit(1);
  }
  exit(0);
}

static int
fmtsize(char f)
{
  switch (f) {
  case 'c':
    return 1;
  case 'h':
    return 2;
  case 'i':
    return 4;
  case 'p':
  case 's':
    return 8;
  default:
    return -1;
  }
}

void
memdump(char *fmt, char *data, int len)
{
  int off = 0;
  int size;
  short h;
  int i;
  uint64 p;
  char *s;

  if (fmt == 0 || (data == 0 && len > 0) || len < 0)
    return;

  // memcpy into locals since data may be unaligned.
  for (; *fmt; fmt++) {
    if (*fmt == 'S') {
      for (; off < len && data[off] != '\0'; off++)
        printf("%c", data[off]);
      printf("\n");
      return;
    }

    if ((size = fmtsize(*fmt)) < 0) {
      printf("memdump: unknown format '%c'\n", *fmt);
      return;
    }
    if (len - off < size) {
      printf("memdump: not enough data for '%c'\n", *fmt);
      return;
    }

    switch (*fmt) {
    case 'c':
      printf("%c\n", data[off]);
      break;
    case 'h':
      memcpy(&h, data + off, sizeof(h));
      printf("%d\n", h);
      break;
    case 'i':
      memcpy(&i, data + off, sizeof(i));
      printf("%d\n", i);
      break;
    case 'p':
      memcpy(&p, data + off, sizeof(p));
      printf("%lx\n", p);
      break;
    case 's':
      memcpy(&s, data + off, sizeof(s));
      printf("%s\n", s ? s : "(null)");
      break;
    }
    off += size;
  }
}
