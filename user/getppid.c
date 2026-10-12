#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  int parent_pid = getppid();

  if (parent_pid <= 0) {
    printf("getppid() returned %d, expected a positive parent PID\n", parent_pid);
    exit(1);
  }

  printf("getppid() returned %d\n", parent_pid);
  return 0;
}
