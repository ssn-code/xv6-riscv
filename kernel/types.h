typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;

typedef uint64 pde_t;

// Process information copied from the kernel to a getproc() caller.
struct uproc {
  int pid;
  int ppid;
  int state;
  uint64 sz;
  char name[16];
};

// System-wide values copied from the kernel to a sysinfo() caller.
struct sysinfo {
  uint64 freemem;
  uint64 nproc;
};

// Per-syscall counters returned by history() to a user program.
struct syscall_stat {
  uint64 count; // Number of completed calls.
  uint64 ticks; // Total timer ticks spent in the kernel handlers.
};
