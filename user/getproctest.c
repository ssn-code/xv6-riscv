#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct uproc info;
  // With no argument, inspect this test process itself.
  int pid = getpid();

  if (argc > 1) {
    pid = atoi(argv[1]);
  }

  // getproc() asks the kernel to copy a process snapshot into info.
  printf("Testing getproc for PID %d...\n", pid);
  if (getproc(pid, &info) == 0) {
    printf("getproc succeeded:\n");
    printf("  PID: %d\n", info.pid);
    printf("  PPID: %d\n", info.ppid);
    printf("  State: %d\n", info.state);
    printf("  Size: %lud bytes\n", info.sz);
    printf("  Name: %s\n", info.name);
  } else {
    printf("getproc failed for PID %d\n", pid);
  }

  // Test with invalid PID
  if (getproc(9999, &info) < 0) {
    printf("getproc correctly failed for non-existent PID 9999\n");
  } else {
    printf("ERROR: getproc succeeded for non-existent PID!\n");
  }

  exit(0);
}
