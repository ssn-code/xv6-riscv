#include "kernel/types.h"
#include "user/user.h"

static char *states[] = {
  [0] "UNUSED  ",
  [1] "USED    ",
  [2] "SLEEPING",
  [3] "RUNNABLE",
  [4] "RUNNING ",
  [5] "ZOMBIE  "
};

int
main(int argc, char *argv[])
{
  struct uproc info;
  int pid;
  int count = 0;

  printf("PID\tPPID\tSTATE\t\tSIZE\tNAME\n");
  printf("---------------------------------------------------\n");

  for (pid = 1; pid <= 256; pid++) {
    if (getproc(pid, &info) == 0) {
      char *st = (info.state >= 0 && info.state <= 5) ? states[info.state] : "UNKNOWN ";
      printf("%d\t%d\t%s\t%lud\t%s\n", info.pid, info.ppid, st, info.sz, info.name);
      count++;
    }
  }

  printf("Total active processes: %d\n", count);
  exit(0);
}
