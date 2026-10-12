#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  int result = hello();

  if (result != 42) {
    printf("hello() returned %d, expected 42\n", result);
    exit(1);
  }

  printf("hello() returned %d\n", result);
  return 0;
}
