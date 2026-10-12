#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  struct sysinfo info;

  // sysinfo() fills this user-space structure with a kernel snapshot.
  if (sysinfo(&info) < 0) {
    fprintf(2, "sysinfotest: sysinfo failed\n");
    exit(1);
  }

  printf("Free memory: %ld bytes\n", info.freemem);
  printf("Active processes: %ld\n", info.nproc);
  exit(0);
}
