#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(2, "usage: trace mask command [args ...]\n");
    exit(1);
  }

  // Set the inherited mask, then replace this process with the command.
  if (trace(atoi(argv[1])) < 0) {
    fprintf(2, "trace: syscall failed\n");
    exit(1);
  }
  exec(argv[2], &argv[2]);
  fprintf(2, "trace: cannot run %s\n", argv[2]);
  exit(1);
}
