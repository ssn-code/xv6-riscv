# Custom System Calls in xv6-RISCV

This document describes the custom system calls and user-space commands added to this xv6-RISCV codebase. It also explains which commands use the new syscalls and which use xv6's existing file-system interface.

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

### `sysinfo()` (Syscall #25)
- **Signature (User):** `int sysinfo(struct sysinfo *info)`
- **Syscall Number:** `SYS_sysinfo` (25)
- **Description:** Returns a snapshot of system-wide statistics. `freemem` is the number of free physical-memory bytes available in the page allocator; `nproc` is the number of active (non-`UNUSED`) process-table entries. Returns `0` on success or `-1` when the result cannot be copied to the supplied user address.
- **User Program:** `sysinfotest` (`user/sysinfotest.c`)

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
| **`kernel/syscall.h`** | Added syscall numbers `SYS_hello` (23), `SYS_getproc` (24), and `SYS_sysinfo` (25). |
| **`kernel/syscall.c`** | Declared syscall handlers and added `hello`, `getproc`, and `sysinfo` to the `syscalls[]` dispatch table. |
| **`kernel/defs.h`** | Declared `kgetproc()`, `freemem()`, and `nproc()` for use across kernel files. |
| **`kernel/sysproc.c`** | Implemented `sys_hello()`, `sys_getproc()`, and `sys_sysinfo()`. `sys_sysinfo()` collects memory and process counts, then copies the result to user memory. |
| **`user/user.h`** | Added user-level prototypes for `hello()`, `getproc()`, and `sysinfo()`. |
| **`user/usys.pl`** | Added entries to auto-generate assembly stubs for `hello`, `getproc`, and `sysinfo`. |
| **`user/hello.c`** | *(New File)* User program demonstrating the `hello()` system call. |
| **`user/getproctest.c`** | *(New File)* User program testing `getproc()` for both valid process PIDs and non-existent PIDs. |
| **`user/sysinfotest.c`** | *(New File)* User program that prints the `sysinfo()` result. |
| **`user/ps.c`** | *(New File)* Process-listing command built on `getproc()`. |
| **`user/edit.c`** | *(New File)* Interactive file editor built with existing file syscalls. |
| **`Makefile`** | Added `$U/_hello`, `$U/_getproctest`, `$U/_sysinfotest`, `$U/_ps`, and `$U/_edit` to `UPROGS` so the programs are compiled and included in `fs.img`. |

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
