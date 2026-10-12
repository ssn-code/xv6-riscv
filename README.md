# Custom System Calls in xv6-RISCV

This document describes the custom system calls and user-space commands added to this xv6-RISCV codebase. It also explains which commands use the new syscalls and which use xv6's existing file-system interface.

---

## 1. Added System Calls

### `hello()` (Syscall #23)
- **Signature (User):** `int hello(void)`
- **Syscall Number:** `SYS_hello` (23)
- **Description:** Prints `"Hello from the kernel!"` to the kernel console and returns `42` to the calling process.
- **User Program:** `hello` (`user/hello.c`)

### `getproc()` (Syscall #24)
- **Signature (User):** `int getproc(int pid, struct uproc *up)`
- **Syscall Number:** `SYS_getproc` (24)
- **Description:** Queries process information for the process matching the given `pid`. On success, populates the user-provided `struct uproc` with process details (`pid`, `ppid`, `state`, memory size `sz`, and process `name`), returning `0`. Returns `-1` if the process is not found or if copying memory fails.
- **User Program:** `getproctest` (`user/getproctest.c`)

### `sysinfo()` (Syscall #25)
- **Signature (User):** `int sysinfo(struct sysinfo *info)`
- **Syscall Number:** `SYS_sysinfo` (25)
- **Description:** Returns a snapshot of system-wide statistics. `freemem` is the number of free physical-memory bytes available in the page allocator; `nproc` is the number of active (non-`UNUSED`) process-table entries. Returns `0` on success or `-1` when the result cannot be copied to the supplied user address.
- **User Program:** `sysinfotest` (`user/sysinfotest.c`)

### `getppid()` (Syscall #26)
- **Signature (User):** `int getppid(void)`
- **Syscall Number:** `SYS_getppid` (26)
- **Description:** Returns the current process's parent PID, or `-1` if the process has no parent.
- **User Program:** `getppid` (`user/getppid.c`)

In this version, `hello()` also differs from its original implementation: `sys_hello()` prints the kernel greeting and returns `42` instead of `0`.

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

### `struct sysinfo` (`kernel/types.h`)
```c
struct sysinfo {
  uint64 freemem;  // Free physical memory in bytes
  uint64 nproc;    // Active process-table entries
};
```

---

## 3. Added User Commands

| Command | Purpose | Relationship to syscalls |
| :--- | :--- | :--- |
| `hello` | Calls `hello()` and demonstrates kernel-to-console output. | Uses the new `hello()` syscall. |
| `getppid` | Prints the parent PID of the running command process. | Uses the new `getppid()` syscall. |
| `getproctest [pid]` | Looks up one process and also checks that an invalid PID fails. | Uses the new `getproc()` syscall. |
| `sysinfotest` | Prints free physical memory and the number of active processes. | Uses the new `sysinfo()` syscall. |
| `ps` | Lists active processes with PID, parent PID, state, memory size, and name. | Calls `getproc()` once for each possible PID. |
| `edit <filename>` | A small interactive text editor. It loads a file, appends entered lines, saves with `:s`, prints with `:p`, clears with `:c`, and exits with `:q`. | Does **not** add a syscall. It uses existing `open`, `read`, `write`, and `close` syscalls. |

`ps` is new in this project; it was not an existing xv6 command in this repository.

---

## 4. Command Reference

### `hello`

**Usage:** `hello`

This is the smallest syscall demonstration program. It invokes `hello()`, which transfers control to `sys_hello()` in the kernel. The kernel prints `Hello from the kernel!` to the xv6 console, then the command exits.

The syscall returns `42`; `hello` checks this value, prints `hello() returned 42`, and exits with an error status if the value differs.

### `getppid`

**Usage:** `getppid`

Prints the parent PID returned by `getppid()`. The test program reports an error if the result is not positive. The exact PID depends on the process that launches the command.

### `getproctest [pid]`

**Usage:** `getproctest` or `getproctest <pid>`

Without an argument, the command queries information about itself. With a PID, it queries that process instead. It prints the returned PID, parent PID, process state, memory size, and name. It also always queries PID `9999` to demonstrate that `getproc()` returns `-1` when no matching process exists.

### `ps`

**Usage:** `ps`

This command is a simple process viewer. It checks PIDs 1 through 256, calls `getproc()` for each PID, and prints one row for every active process. The output includes PID, parent PID, readable process state, memory size, and program name. Its process information comes entirely from the new `getproc()` syscall.

### `sysinfotest`

**Usage:** `sysinfotest`

This command calls `sysinfo()` and prints the returned system snapshot. `Free memory` is calculated by walking the allocator's free-page list while holding its lock. `Active processes` is calculated by checking each process-table entry while holding that process's lock.

### `edit <filename>`

**Usage:** `edit <filename>`

This command is a small line-based text editor. It loads the named file when it exists; otherwise, it starts with an empty buffer and creates the file when it is saved. Any normal text entered at the `edit>` prompt is appended as a new line.

| Input | Action |
| :--- | :--- |
| `:s` | Save the current buffer to the named file. |
| `:p` | Print the editor interface and current buffer. |
| `:c` | Clear the in-memory buffer. |
| `:q` | Exit the editor. |

`edit` does not introduce a new kernel syscall. It implements file editing with xv6's existing `open`, `read`, `write`, and `close` syscalls.

---

## 5. Kernel and User-Space Changes

Below is the complete trace of files modified and added for the syscalls and commands:

| File Path | Description of Edits |
| :--- | :--- |
| **`kernel/types.h`** | Added `struct uproc` definition so it is available across kernel and user space. |
| **`kernel/kalloc.c`** | Added `freemem()` to count free physical-memory bytes while holding the allocator lock. |
| **`kernel/proc.c`** | Implemented `kgetproc(int pid, uint64 up)` to scan the `proc[NPROC]` table under `p->lock`, populate `struct uproc`, and safely copy data to user memory using `copyout()`. Added `nproc()` to count active process-table entries safely. |
| **`kernel/syscall.h`** | Defines custom syscall numbers `SYS_hello` (23), `SYS_getproc` (24), `SYS_sysinfo` (25), and `SYS_getppid` (26). |
| **`kernel/syscall.c`** | Declares custom syscall handlers and registers `hello`, `getproc`, `sysinfo`, and `getppid` in the `syscalls[]` dispatch table. |
| **`kernel/defs.h`** | Declared `kgetproc()`, `freemem()`, and `nproc()` for use across kernel files. |
| **`kernel/sysproc.c`** | Implements `sys_hello()`, `sys_getproc()`, `sys_sysinfo()`, and `sys_getppid()`. `sys_sysinfo()` collects memory and process counts, then copies the result to user memory. |
| **`user/user.h`** | Declares user-level prototypes for `hello()`, `getproc()`, `sysinfo()`, and `getppid()`. |
| **`user/usys.pl`** | Generates assembly stubs for `hello`, `getproc`, `sysinfo`, and `getppid`. |
| **`user/hello.c`** | *(New File)* User program demonstrating the `hello()` system call. |
| **`user/getppid.c`** | *(New File)* User program testing that `getppid()` returns a positive parent PID. |
| **`user/getproctest.c`** | *(New File)* User program testing `getproc()` for both valid process PIDs and non-existent PIDs. |
| **`user/sysinfotest.c`** | *(New File)* User program that prints the `sysinfo()` result. |
| **`user/ps.c`** | *(New File)* Process-listing command built on `getproc()`. |
| **`user/edit.c`** | *(New File)* Interactive file editor built with existing file syscalls. |
| **`Makefile`** | Adds `$U/_hello`, `$U/_getppid`, `$U/_getproctest`, `$U/_sysinfotest`, `$U/_ps`, and `$U/_edit` to `UPROGS` so the programs are compiled and included in `fs.img`. |

---

## 6. How to Build and Run

To compile the kernel, build the disk image, and run in QEMU:

```bash
make clean
make qemu
```

Inside the xv6 shell:

```sh
$ hello
Hello from the kernel!
hello() returned 42

$ getppid
getppid() returned <positive parent PID>

$ getproctest
Testing getproc for PID 3...
getproc succeeded:
  PID: 3
  PPID: 2
  State: 4
  Size: 16384d bytes
  Name: getproctest
getproc correctly failed for non-existent PID 9999

$ sysinfotest
Free memory: 126976000 bytes
Active processes: 3

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

## 7. `hello()` System Call Lab

This walkthrough follows the `hello()` syscall from a user program into the kernel and back. The syscall is intentionally small: it prints a kernel message and returns the integer `42`.

### Learning goals

- Assign and register a unique syscall number.
- Trace the user-space wrapper, `ecall`, kernel trap, and dispatch table.
- Implement a kernel handler and expose it to user programs.
- Build xv6, run it in QEMU, and verify the return value.

### Call flow

1. `user/hello.c` calls `hello()`.
2. The generated user stub places `SYS_hello` in `a7` and executes `ecall`.
3. The trap handler transfers control to `syscall()`, which dispatches through the syscall table.
4. `sys_hello()` prints the greeting and returns `42` in `a0`.
5. The user stub returns the value to `hello.c`, which checks it.

### Implementation files

| File | Role |
| :--- | :--- |
| `kernel/syscall.h` | Defines the unique syscall number, currently `SYS_hello` as `23`. |
| `kernel/syscall.c` | Declares `sys_hello` and maps `SYS_hello` to it in the dispatch table. |
| `kernel/sysproc.c` | Implements `sys_hello()`, which prints the greeting and returns `42`. |
| `user/usys.pl` | Generates the user-space assembly stub; this is the source to edit, not `user/usys.S`. |
| `user/user.h` | Declares the user-facing `int hello(void)` function. |
| `user/hello.c` | Calls the syscall and fails if the result is not `42`. |
| `Makefile` | Includes `_hello` in the user programs packaged into `fs.img`. |

### Toolchain prerequisites

Install Git, GNU Make, QEMU 7.2 or newer, and a RISC-V cross-compiler with binutils. The Makefile recognizes these compiler prefixes:

- `riscv64-unknown-elf-`
- `riscv64-elf-`
- `riscv64-none-elf-`
- `riscv64-linux-gnu-`
- `riscv64-unknown-linux-gnu-`

On Ubuntu or Debian, the usual packages are:

```sh
sudo apt update
sudo apt install -y build-essential git make qemu-system-misc bc
sudo apt install -y gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu
```

If automatic toolchain detection does not find the compiler, pass its prefix explicitly, for example:

```sh
make TOOLPREFIX=riscv64-linux-gnu- qemu
```

### Build and verify

From the repository root, build the kernel and user programs with `make`. To rebuild just the hello program and its generated syscall stub, run:

```sh
make user/_hello
```

The Makefile regenerates `user/usys.S` from `user/usys.pl`. To inspect the stub:

```sh
grep -A4 -B1 '^hello:' user/usys.S
```

It should load `SYS_hello` into `a7`, execute `ecall`, and return. Start xv6 with `make qemu`, then run `hello` in the xv6 shell. Expected output includes:

```text
Hello from the kernel!
hello() returned 42
```

The kernel message appears on the console, and the command exits successfully only when the syscall returns exactly `42`.

### Troubleshooting

- **`hello` is not found:** Run `make clean` and `make`, and confirm `_hello` is listed in `UPROGS` in the Makefile.
- **Undefined reference to `hello`:** Confirm the declaration is in `user/user.h` and `entry("hello");` is in `user/usys.pl`, then rebuild `user/_hello`.
- **Unknown syscall:** Confirm `SYS_hello` is unique and that `sys_hello` is both declared and registered in `kernel/syscall.c`; rebuild the kernel and restart QEMU.
- **The program reports a value other than `42`:** Confirm `sys_hello()` returns `42` and the generated stub uses `SYS_hello`.

### Extension exercises

1. Change the syscall to return the current process ID.
2. Add an argument and return the value supplied by the caller.
3. Call `hello()` from a child process created with `fork()`.
4. Add a syscall that accepts a string and prints it through the kernel log.
5. Test the syscall from multiple processes.

For each exercise, update the syscall number, kernel declaration and dispatch entry, handler, generated user stub, user declaration, and test program as needed.

## 8. System Call Development and `getppid()` Lab

### System call path

A user program's ordinary C call reaches the kernel through a generated RISC-V stub. The syscall number is passed in register `a7`; the return value comes back in `a0`.

1. The user program calls a function declared in `user/user.h`.
2. `user/usys.pl` generates its assembly stub in `user/usys.S`.
3. The stub loads the syscall number into `a7` and executes `ecall`.
4. The trap code enters `syscall()` in `kernel/syscall.c`.
5. The dispatcher indexes `syscalls[]`, calls the registered handler, and stores its result in `a0`.
6. The user stub returns to the program with that result.

The `getpid()` path is a useful reference for a simple no-argument syscall. For a new syscall, update the number, kernel implementation, handler declaration and dispatch entry, user declaration, generated stub entry, test program, and `UPROGS` list.

### `getppid()` implementation

`getppid()` reads the parent relationship already stored in `struct proc`. `kfork()` assigns `np->parent = p` when it creates a child; no new process fields are needed.

The syscall number is `26` because this repository already assigns `23` to `hello`, `24` to `getproc`, and `25` to `sysinfo`:

```c
#define SYS_getppid 26
```

The kernel handler in `kernel/sysproc.c` is:

```c
uint64
sys_getppid(void)
{
  struct proc *p = myproc();

  return p->parent ? p->parent->pid : -1;
}
```

Register `sys_getppid()` in `kernel/syscall.c` with its declaration and dispatch entry:

```c
extern uint64 sys_getppid(void);
```

```c
[SYS_getppid] = sys_getppid,
```

Expose the call to user programs and generate its wrapper:

```c
int getppid(void);
```

```perl
entry("getppid");
```

The generated stub loads `SYS_getppid` into `a7`, executes `ecall`, and returns. The handler returns `-1` when the parent pointer is null; otherwise, it returns the parent's PID.

The test program in `user/getppid.c` calls `getppid()`, fails if the result is not positive, and prints the returned PID. The Makefile includes `$U/_getppid` in `UPROGS`, making the command available in the xv6 shell.

### Build and test

From the repository root, build the kernel and user programs, then start xv6:

```sh
make clean
make
make qemu
```

Run `getppid` in the xv6 shell. Expected output has the form:

```text
getppid() returned <positive parent PID>
```

The specific PID is runtime-dependent. To verify the generated wrapper on the host, run:

```sh
grep -A4 -B1 '^getppid:' user/usys.S
```

The stub should use `SYS_getppid`, execute `ecall`, and return. A child-process test can additionally compare the value returned in the child with the PID of the process that forked it.

### Common syscall integration errors

| Symptom | Check |
| :--- | :--- |
| `getppid` is not found | Confirm `$U/_getppid` is in `UPROGS`, then rebuild. |
| Undefined reference or implicit declaration | Confirm `int getppid(void);` is in `user/user.h` and `entry("getppid");` is in `user/usys.pl`. |
| `SYS_getppid` is undeclared | Define the unique syscall number in `kernel/syscall.h`. |
| Unknown syscall 26 | Confirm the handler declaration and `[SYS_getppid]` dispatch entry in `kernel/syscall.c`. |
| Non-positive result | Confirm the handler returns the parent's PID and that the current process has a parent. |

The same integration pattern applies to the other custom calls documented above. Keep syscall numbers unique, regenerate `user/usys.S` from `user/usys.pl`, and rebuild after changing kernel code.
