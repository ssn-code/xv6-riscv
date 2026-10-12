#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

static struct spinlock syscall_stats_lock;
static struct syscall_stat syscall_stats[SYS_sleep + 1];

// Names are used by trace() and must match the syscall number table.
static char *syscall_names[] = {
  [SYS_fork] = "fork", [SYS_exit] = "exit", [SYS_wait] = "wait",
  [SYS_pipe] = "pipe", [SYS_read] = "read", [SYS_kill] = "kill",
  [SYS_exec] = "exec", [SYS_fstat] = "fstat", [SYS_chdir] = "chdir",
  [SYS_dup] = "dup", [SYS_getpid] = "getpid", [SYS_sbrk] = "sbrk",
  [SYS_pause] = "pause", [SYS_uptime] = "uptime", [SYS_open] = "open",
  [SYS_write] = "write", [SYS_mknod] = "mknod", [SYS_unlink] = "unlink",
  [SYS_link] = "link", [SYS_mkdir] = "mkdir", [SYS_close] = "close",
  [SYS_sync] = "sync", [SYS_hello] = "hello", [SYS_getproc] = "getproc",
  [SYS_sysinfo] = "sysinfo", [SYS_getppid] = "getppid",
  [SYS_trace] = "trace", [SYS_history] = "history", [SYS_sleep] = "sleep",
};

// Initialize the lock that protects global syscall history counters.
void
syscallinit(void)
{
  initlock(&syscall_stats_lock, "syscall_stats");
}

// Copy one statistic while holding its lock; used by sys_history().
int
getsyscallstat(int num, struct syscall_stat *stat)
{
  if (num <= 0 || num >= NELEM(syscall_stats) || syscall_names[num] == 0)
    return -1;
  acquire(&syscall_stats_lock);
  *stat = syscall_stats[num];
  release(&syscall_stats_lock);
  return 0;
}

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if (addr >= p->sz ||
      addr + sizeof(uint64) > p->sz) // both tests needed, in case of overflow
    return -1;
  if (copyin(p->pagetable, p->sz, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  if (copyinstr(p->pagetable, p->sz, buf, addr, max) < 0)
    return -1;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
void
argint(int n, int *ip)
{
  *ip = argraw(n);
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
void
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (not including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  argaddr(n, &addr);
  return fetchstr(addr, buf, max);
}

// Prototypes for the functions that handle system calls.
extern uint64 sys_fork(void);
extern uint64 sys_exit(void);
extern uint64 sys_wait(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_kill(void);
extern uint64 sys_exec(void);
extern uint64 sys_fstat(void);
extern uint64 sys_chdir(void);
extern uint64 sys_dup(void);
extern uint64 sys_getpid(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_pause(void);
extern uint64 sys_uptime(void);
extern uint64 sys_open(void);
extern uint64 sys_write(void);
extern uint64 sys_mknod(void);
extern uint64 sys_unlink(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_close(void);
extern uint64 sys_sync(void);
// Handlers for the custom syscall numbers declared in syscall.h.
extern uint64 sys_hello(void);
extern uint64 sys_getproc(void);
extern uint64 sys_sysinfo(void);
extern uint64 sys_getppid(void);
extern uint64 sys_trace(void);
extern uint64 sys_history(void);
extern uint64 sys_sleep(void);

// An array mapping syscall numbers from syscall.h
// to the function that handles the system call.
static uint64 (*syscalls[])(void) = {
  // clang-format off
  [SYS_fork]    = sys_fork,
  [SYS_exit]    = sys_exit,
  [SYS_wait]    = sys_wait,
  [SYS_pipe]    = sys_pipe,
  [SYS_read]    = sys_read,
  [SYS_kill]    = sys_kill,
  [SYS_exec]    = sys_exec,
  [SYS_fstat]   = sys_fstat,
  [SYS_chdir]   = sys_chdir,
  [SYS_dup]     = sys_dup,
  [SYS_getpid]  = sys_getpid,
  [SYS_sbrk]    = sys_sbrk,
  [SYS_pause]   = sys_pause,
  [SYS_uptime]  = sys_uptime,
  [SYS_open]    = sys_open,
  [SYS_write]   = sys_write,
  [SYS_mknod]   = sys_mknod,
  [SYS_unlink]  = sys_unlink,
  [SYS_link]    = sys_link,
  [SYS_mkdir]   = sys_mkdir,
  [SYS_close]   = sys_close,
  [SYS_sync]    = sys_sync,
  // Connect each custom syscall number to its kernel implementation.
  [SYS_hello]   = sys_hello,
  [SYS_getproc] = sys_getproc,
  [SYS_sysinfo] = sys_sysinfo,
  [SYS_getppid] = sys_getppid,
  [SYS_trace]   = sys_trace,
  [SYS_history] = sys_history,
  [SYS_sleep]   = sys_sleep,
  // clang-format on
};


void
syscall(void)
{
  int num;
  uint start_ticks;
  uint elapsed_ticks;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    // Measure only time spent in the kernel handler, in timer ticks.
    acquire(&tickslock);
    start_ticks = ticks;
    release(&tickslock);

    // Use num to lookup the system call function for num, call it,
    // and store its return value in p->trapframe->a0
    p->trapframe->a0 = syscalls[num]();

    acquire(&tickslock);
    elapsed_ticks = ticks - start_ticks;
    release(&tickslock);

    acquire(&syscall_stats_lock);
    syscall_stats[num].count++;
    syscall_stats[num].ticks += elapsed_ticks;
    release(&syscall_stats_lock);

    // trace() prints completed calls selected by this process's bitmask.
    if (p->trace_mask & (1U << num))
      printk("%d: syscall %s -> %ld\n", p->pid, syscall_names[num],
             p->trapframe->a0);
  } else {
    printk("%d %s: unknown sys call %d\n", p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
