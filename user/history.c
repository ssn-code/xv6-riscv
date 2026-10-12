#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct syscall_stat stat;
  int num;

  if (argc != 2) {
    fprintf(2, "usage: history syscall-number\n");
    exit(1);
  }
  num = atoi(argv[1]);
  if (history(num, &stat) < 0) {
    fprintf(2, "history: invalid syscall number %d\n", num);
    exit(1);
  }
  printf("Syscall %d\n", num);
  printf("Count: %ld\n", stat.count);
  printf("Kernel ticks: %ld\n", stat.ticks);
  exit(0);
}
