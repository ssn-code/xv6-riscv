# Custom System Calls in xv6-RISCV

This document describes the custom system calls added to this xv6-RISCV codebase, along with a detailed list of all relative file modifications across the kernel and user space.

---

## 1. Added System Calls

### `hello()` (Syscall #23)
- **Signature (User):** `int hello(void)`
- **Syscall Number:** `SYS_hello` (23)
- **Description:** Prints a greeting message `"Hello from the kernel!"` to the kernel console output.
- **User Program:** `hello` (`user/hello.c`)

### `getproc()` (Syscall #24)
- **Signature (User):** `int getproc(int pid, struct uproc *up)`
- **Syscall Number:** `SYS_getproc` (24)
- **Description:** Queries process information for the process matching the given `pid`. On success, populates the user-provided `struct uproc` with process details (`pid`, `ppid`, `state`, memory size `sz`, and process `name`), returning `0`. Returns `-1` if the process is not found or if copying memory fails.
- **User Program:** `getproctest` (`user/getproctest.c`)

---

## 2. Structure Definitions

### `struct uproc` (`kernel/types.h`)
```c
struct uproc {
  int pid;         // Process ID
  int ppid;        // Parent Process ID
  int state;       // Process State (enum procstate)
  uint64 sz;       // Memory size of process in bytes
  char name[16];   // Process name
};
```

---

## 3. Relative Edits Across Codebase Files

Below is the complete trace of files modified and added to implement these system calls:

| File Path | Description of Edits |
| :--- | :--- |
| **`kernel/types.h`** | Added `struct uproc` definition so it is available across kernel and user space. |
| **`kernel/syscall.h`** | Added syscall numbers `#define SYS_hello 23` and `#define SYS_getproc 24`. |
| **`kernel/syscall.c`** | Declared `sys_hello` and `sys_getproc` prototypes, and added `[SYS_hello]` / `[SYS_getproc]` mappings to the `syscalls[]` function pointer array. |
| **`kernel/defs.h`** | Added kernel function declaration `int kgetproc(int, uint64);` under the `proc.c` section. |
| **`kernel/sysproc.c`** | Implemented `sys_hello(void)` and `sys_getproc(void)` system call wrapper functions. `sys_getproc` extracts arguments using `argint()` and `argaddr()`. |
| **`kernel/proc.c`** | Implemented `kgetproc(int pid, uint64 up)` to scan the `proc[NPROC]` table under `p->lock`, populate `struct uproc`, and safely copy data to user memory using `copyout()`. |
| **`user/user.h`** | Added user-level function prototypes `int hello(void);` and `int getproc(int, struct uproc *);`. |
| **`user/usys.pl`** | Added `entry("hello");` and `entry("getproc");` to auto-generate Assembly stubs in `usys.S`. |
| **`user/hello.c`** | *(New File)* User program demonstrating the `hello()` system call. |
| **`user/getproctest.c`** | *(New File)* User program testing `getproc()` for both valid process PIDs and non-existent PIDs. |
| **`Makefile`** | Added `$U/_hello` and `$U/_getproctest` to `UPROGS` so user binaries are compiled and included in `fs.img`. |

---

## 4. How to Build and Run

To compile the kernel, build the disk image, and run in QEMU:

```bash
make clean
make qemu
```

Inside the xv6 shell:

```sh
$ hello
Hello from the kernel!

$ getproctest
Testing getproc for PID 3...
getproc succeeded:
  PID: 3
  PPID: 2
  State: 4
  Size: 16384d bytes
  Name: getproctest
getproc correctly failed for non-existent PID 9999

$ ps
PID	PPID	STATE		SIZE	NAME
---------------------------------------------------
1	0	SLEEPING	16384d	init
2	1	SLEEPING	20480d	sh
3	2	RUNNING 	16384d	ps
Total active processes: 3

$ edit notes.txt
[Status] File 'notes.txt' does not exist yet. Creating a new file on save.
edit> Hello xv6 text editor!
edit> Line 2 of notes.
edit> :s
[Status] File 'notes.txt' saved successfully (40 bytes).
edit> :q
Exiting editor.

$ cat notes.txt
Hello xv6 text editor!
Line 2 of notes.
```


