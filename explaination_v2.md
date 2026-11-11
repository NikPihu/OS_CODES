# OS Laboratory — Explanation Guide v2 (Deep Theory + Every Line + Call Flow)

> **Code covered:** the 9 files in `short_corrected_code/` (this same folder).
> **Line numbers** in this guide match those files exactly, so you can keep the code open side by side.
> **How each assignment is laid out:**
>
> | Part | What it gives you |
> |---|---|
> | **A. Aim** | What the program is supposed to achieve, in plain words |
> | **B. Theory** | The OS concepts you must be able to explain in a viva |
> | **C. Functions & System Calls** | Every library/system call used: header, prototype, return value |
> | **D. Execution Flow (Call Chain)** | Exactly which function calls which, in what order, in which process/thread |
> | **E. Line-by-Line Explanation** | Every single line of the code explained |
> | **F. Dry Run** | A full trace with sample input → exact output |
> | **G. Compile & Run** | Commands to run it |
> | **H. Mistakes, Edge Cases & Viva** | What examiners ask and what breaks |

---

## Table of Contents

0. [Before You Start: Common Background](#0-before-you-start-common-background)
1. [Assignment 1 — Shell Script: Address Book](#assignment-1--shell-script-address-book)
2. [Assignment 2 — Process Management: fork, wait, Zombie, Orphan](#assignment-2--process-management-fork-wait-zombie-orphan)
3. [Assignment 3 — Thread Synchronization: Producer–Consumer](#assignment-3--thread-synchronization-producerconsumer)
4. [Assignment 4 — Deadlock Avoidance: Dining Philosophers](#assignment-4--deadlock-avoidance-dining-philosophers)
5. [Assignment 5 — Deadlock Avoidance: Banker's Algorithm](#assignment-5--deadlock-avoidance-bankers-algorithm)
6. [Assignment 6 — IPC: Full-Duplex Communication using FIFOs](#assignment-6--ipc-full-duplex-communication-using-fifos)
7. [Assignment 7 — IPC: Full-Duplex Communication using Pipes](#assignment-7--ipc-full-duplex-communication-using-pipes)
8. [Assignment 8 — Disk Scheduling: SSTF, SCAN, C-LOOK](#assignment-8--disk-scheduling-sstf-scan-c-look)
9. [Quick Revision Sheet](#9-quick-revision-sheet)

---

# 0. Before You Start: Common Background

These ideas show up in almost every assignment. Learn them once.

### 0.1 Program vs Process vs Thread

| Term | Meaning | Example in our code |
|---|---|---|
| **Program** | A file of instructions on disk (passive) | `2_process.c` compiled into `a.out` |
| **Process** | A program *in execution*: own memory (code, data, heap, stack), own PID, own open files | Each `fork()` creates a new process |
| **Thread** | A unit of execution *inside* a process. Threads of one process **share** global variables, heap and open files, but each has its **own stack and registers** | `producer` and `consumer` threads in Assignment 3 |

### 0.2 Memory Layout of a Process

```
 High address
 ┌──────────────────────┐
 │ Stack                │  local variables, function call frames (grows down ↓)
 │          ↓           │
 │                      │
 │          ↑           │
 │ Heap                 │  malloc / new (grows up ↑)
 ├──────────────────────┤
 │ BSS                  │  uninitialised globals (e.g. int buffer[5];)
 │ Data                 │  initialised globals (e.g. int in = 0;)
 │ Text (Code)          │  machine instructions (read-only)
 └──────────────────────┘
 Low address
```

- After `fork()`, the child gets a **copy** of all of this (implemented using **Copy-On-Write**: pages are shared until someone writes, then copied).
- Threads (from `pthread_create`) do **not** copy anything — they share Text, Data, BSS and Heap, but each gets its own Stack.

### 0.3 System Call vs Library Function

- **System call**: a request to the OS kernel (switches CPU from *user mode* to *kernel mode*). Examples: `fork`, `wait`, `pipe`, `read`, `write`, `open`, `close`, `mkfifo`, `getpid`.
- **Library function**: ordinary code in a library (libc, libpthread) that may or may not call the kernel internally. Examples: `printf`, `scanf`, `fopen`, `fread`, `abs`, `sem_wait`, `pthread_create`.

### 0.4 File Descriptors (needed for Assignments 6 & 7)

Every process has a table of open files. Each entry is identified by a small integer — the **file descriptor (fd)**.

| fd | Name | Default |
|---|---|---|
| 0 | `stdin` | keyboard |
| 1 | `stdout` | screen |
| 2 | `stderr` | screen |
| 3, 4, … | next files you open | given out in increasing order |

`open()`, `pipe()` return fds; `read(fd, …)` / `write(fd, …)` use them; `close(fd)` releases them.

### 0.5 Process States (5-state model)

```
  New ──admit──► Ready ◄──────── I/O done / event ──────┐
                  │  ▲                                  │
         dispatch │  │ timeout (preempted)              │
                  ▼  │                                  │
                Running ──── wait for I/O / sem_wait ──► Waiting (Blocked)
                  │
                exit()
                  ▼
              Terminated  (becomes a ZOMBIE until the parent calls wait())
```

`sem_wait`, `pthread_mutex_lock`, `wait`, `read` on an empty pipe, `open` on a FIFO, `sleep` — all of these can move a process/thread into the **Waiting** state.

---
---

# Assignment 1 — Shell Script: Address Book

**File:** `1_address_book.sh`

## A. Aim

Write a menu-driven **bash** script that maintains an address book stored in a text file `address.txt`. The user must be able to:

| Option | Operation | What happens to `address.txt` |
|---|---|---|
| `a` / `1` | Create | File is created (or emptied if it exists) |
| `b` / `2` | View | All records are printed |
| `c` / `3` | Insert | A new record `Name Phone City` is appended |
| `d` / `4` | Delete | The record whose name matches is removed |
| `e` / `5` | Modify | The matching record is replaced with new phone/city |
| `f` / `6` | Exit | Script ends |

Each record is **one line** of the file; fields are separated by a **space**:
```
John 9876543210 Pune
Mary 9123456780 Mumbai
```

## B. Theory

### B.1 What is a shell?
- The **shell** is a command interpreter: a program that reads commands you type (or that are written in a script) and asks the kernel to run them.
- **bash** (Bourne Again SHell) is the most common Linux shell.
- A **shell script** is a plain text file containing shell commands. It lets you automate tasks without compiling anything.
- The first line `#!/bin/bash` is the **shebang**: it tells the kernel which interpreter to use when you run `./1_address_book.sh`.

### B.2 Variables in bash
- Created with `name=value` — **no spaces around `=`**.
- Read with `$name`. Always write `"$name"` in double quotes if the value may contain spaces.
- All variables are strings by default.

### B.3 Input and Output
| Command | Meaning |
|---|---|
| `echo "text"` | Print text followed by a newline |
| `echo -e "a\tb\n"` | `-e` enables escape sequences like `\t` (tab) and `\n` (newline) |
| `read var` | Read one line from keyboard into `var` |
| `read -p "Prompt: " var` | Print a prompt first, then read |

### B.4 I/O Redirection
| Operator | Meaning |
|---|---|
| `> file` | Send output to `file`, **truncating** (emptying) it first. Creates it if missing |
| `>> file` | Send output to `file`, **appending** to the end |
| `2>/dev/null` | Send **error messages** (fd 2 = stderr) to `/dev/null`, the "black hole" file that discards everything |
| `cmd1 && cmd2` | Run `cmd2` **only if** `cmd1` succeeded (exit status 0) |

> A command with *only* a redirection and no command, like `> address.txt`, simply creates/empties the file.

### B.5 Exit status — how `if` works in bash
- Every command returns an **exit status**: `0` = success/true, non-zero = failure/false.
- `if cmd; then … fi` runs the `then` part if `cmd` returned `0`.
- That's why we can write `if grep -q …; then` — `grep` returns `0` when it finds a match.

### B.6 Test conditions `[ … ]`
`[` is actually a command (also called `test`). Common file tests:

| Test | True when |
|---|---|
| `[ -e file ]` | file exists |
| `[ -f file ]` | file exists and is a regular file |
| `[ -s file ]` | file exists **and size > 0** (not empty) |
| `[ -r file ]` / `-w` / `-x` | readable / writable / executable |

### B.7 `grep` — searching text with patterns
`grep PATTERN file` prints the lines of `file` that match `PATTERN` (a regular expression).

| Option / Symbol | Meaning |
|---|---|
| `-q` | *quiet*: print nothing, only set exit status (0 = found) |
| `-v` | *invert*: print lines that do **NOT** match |
| `^` | regex anchor: "start of line" |
| `$` (in regex) | regex anchor: "end of line" |

### B.8 Loop and menu constructs
```bash
while true          # true is a command that always returns 0 → infinite loop
do
    ...
done

case $ch in
    a|1) commands ;;   # pattern "a" OR "1"; ;; ends this branch
    *)   commands ;;   # * matches anything → default branch
esac
```

## C. Commands Used

| Command | Purpose in this script |
|---|---|
| `echo` / `echo -e` | Print menu, messages, table header |
| `read -p` | Read menu choice and record fields |
| `>` | Create / empty `address.txt` |
| `>>` | Append a record |
| `[ -s file ]` | Check whether there is anything to show |
| `cat` | Print the whole file |
| `grep -q` | Check whether a name exists |
| `grep -v` | Copy every line *except* the matching one |
| `mv` | Rename `temp.txt` → `address.txt` (replace the old file) |
| `exit 0` | Terminate the script with success status |

## D. Execution Flow

```
START (bash reads the file top to bottom)
  │
  ├─ line 5: file="address.txt"
  │
  └─ line 7: while true ─────────────────────────────────────────────┐
        │                                                            │
        ├─ line 9 : echo menu                                        │
        ├─ line 10: read ch                                          │
        └─ line 12: case $ch in                                      │
              │                                                      │
              ├─ a|1 → [> file]            → "created"         ──────┤
              ├─ b|2 → [-s file]?                                    │
              │          ├─ yes → echo header → cat file       ──────┤
              │          └─ no  → "empty"                      ──────┤
              ├─ c|3 → read name,phone,city → echo >> file     ──────┤
              ├─ d|4 → read name → grep -q?                          │
              │          ├─ found → grep -v > temp → mv        ──────┤
              │          └─ not   → "not found"                ──────┤
              ├─ e|5 → read name → grep -q?                          │
              │          ├─ found → grep -v > temp → mv              │
              │          │          → read phone,city → echo >> ─────┤
              │          └─ not   → "not found"                ──────┤
              ├─ f|6 → "Exiting." → exit 0 ──► END                   │
              └─ *   → "Invalid choice."                       ──────┘
                                                (back to top of loop)
```

**Delete / Modify trick — the data flow:**
```
address.txt ──grep -v "^John "──► temp.txt ──mv──► address.txt
(all lines)    (drop John's line)  (everyone       (old file replaced)
                                    except John)
```

## E. Line-by-Line Explanation

| Line | Code | Explanation |
|---|---|---|
| 1 | `#!/bin/bash` | **Shebang.** Tells the OS to run this file with `/bin/bash`. Without it, `./script` might run under a different shell. |
| 2–3 | `# ...` | Comments (bash ignores everything after `#`). Document the menu options. |
| 5 | `file="address.txt"` | Store the database file name in a variable so it is written in one place only. No spaces around `=`. |
| 7 | `while true` | Start an infinite loop. `true` always exits with 0, so the loop never stops by itself — only `exit` (option f) ends it. |
| 8 | `do` | Beginning of the loop body. |
| 9 | `echo -e "\na)Create ..."` | Print a blank line (`\n`) and then the menu. `-e` is needed so `\n` is treated as a newline, not two characters. |
| 10 | `read -p "Enter choice: " ch` | Show the prompt and store what the user types in variable `ch`. |
| 12 | `case $ch in` | Compare `ch` against each pattern below, top to bottom; run the first one that matches. |
| 13 | `a\|1) > $file` | Pattern `a` **or** `1`. `> $file` with no command **truncates** the file to 0 bytes (creates it if absent). This is "Create". |
| 14 | `echo "Address book created." ;;` | Confirmation. `;;` ends this case branch (like `break` in C). |
| 16 | `b\|2) if [ -s "$file" ]; then` | View. `-s` is true only if the file exists **and is not empty** — so we never print an empty table. |
| 17 | `echo -e "\nName\tPhone\tCity"` | Print a header row separated by tabs. |
| 18 | `cat $file` | Print every line (record) of the file. |
| 19–20 | `else echo "Address book is empty."` | Shown if the file is missing or has 0 bytes. |
| 21 | `fi ;;` | `fi` closes the `if`; `;;` closes the case branch. |
| 23 | `c\|3) read -p "Name: " name` | Insert. Read the name. |
| 24 | `read -p "Phone: " phone` | Read the phone number. |
| 25 | `read -p "City: " city` | Read the city. |
| 26 | `echo "$name $phone $city" >> $file` | Build one line `Name Phone City` and **append** it (`>>`), so old records stay. |
| 27 | `echo "Record inserted." ;;` | Confirmation; end branch. |
| 29 | `d\|4) read -p "Enter name to delete: " name` | Delete. Ask which name. |
| 30 | `if grep -q "^$name " $file 2>/dev/null; then` | Search silently (`-q`) for a line that **starts with** (`^`) the name followed by a **space**. The trailing space makes it an exact first-field match (searching `Al` won't hit `Alex`). `2>/dev/null` hides the error if the file does not exist yet. |
| 31 | `grep -v "^$name " $file > temp.txt && mv temp.txt $file` | `-v` writes **all other lines** into `temp.txt`. If that succeeded (`&&`), `mv` replaces `address.txt` with `temp.txt`. Net effect: the record is deleted. |
| 32 | `echo "Record deleted."` | Confirmation. |
| 33–34 | `else echo "Record not found."` | Name was not in the file. |
| 35 | `fi ;;` | Close if, close branch. |
| 37 | `e\|5) read -p "Enter name to modify: " name` | Modify. Ask which name. |
| 38 | `if grep -q "^$name " $file 2>/dev/null; then` | Same existence check as delete. |
| 39 | `grep -v "^$name " $file > temp.txt && mv temp.txt $file` | **Step 1 of modify = delete the old record.** |
| 40 | `read -p "New Phone: " phone` | Read new phone. |
| 41 | `read -p "New City: " city` | Read new city. |
| 42 | `echo "$name $phone $city" >> $file` | **Step 2 of modify = insert the new record** (same name, new data). Note it moves to the end of the file. |
| 43 | `echo "Record modified."` | Confirmation. |
| 44–46 | `else … fi ;;` | Not found case; close. |
| 48 | `f\|6) echo "Exiting."` | Exit option. |
| 49 | `exit 0 ;;` | Terminate the **whole script** with status 0 (success). This is the only way out of `while true`. |
| 51 | `*) echo "Invalid choice." ;;` | `*` matches anything not matched above → default branch. |
| 52 | `esac` | End of `case` (`case` spelled backwards). |
| 53 | `done` | End of `while`; jump back to line 7. |

## F. Dry Run

| Step | Input | `address.txt` after the step | Screen |
|---|---|---|---|
| 1 | `a` | *(empty file)* | `Address book created.` |
| 2 | `b` | *(empty)* | `Address book is empty.` → `-s` is false |
| 3 | `c`, `John`, `111`, `Pune` | `John 111 Pune` | `Record inserted.` |
| 4 | `c`, `Mary`, `222`, `Mumbai` | `John 111 Pune`<br>`Mary 222 Mumbai` | `Record inserted.` |
| 5 | `e`, `John`, `999`, `Delhi` | `Mary 222 Mumbai`<br>`John 999 Delhi` | `Record modified.` (John moved to the end) |
| 6 | `d`, `Mary` | `John 999 Delhi` | `Record deleted.` |
| 7 | `d`, `Alex` | `John 999 Delhi` | `Record not found.` |
| 8 | `x` | unchanged | `Invalid choice.` |
| 9 | `f` | unchanged | `Exiting.` → script ends |

Detail of step 6 (`grep` view):
```
grep -q "^Mary " address.txt   → finds "Mary 222 Mumbai" → exit status 0 → then-branch
grep -v "^Mary " address.txt   → outputs "John 999 Delhi"  → written to temp.txt
mv temp.txt address.txt         → address.txt now has only John
```

## G. Compile & Run

```bash
chmod +x 1_address_book.sh
```
```bash
./1_address_book.sh
```
(or `bash 1_address_book.sh` — no `chmod` needed then)

## H. Mistakes, Edge Cases & Viva

**Edge cases in this code**
- A name with a space (e.g. `John Smith`) breaks the "first field = name" rule, because fields are space-separated.
- The header uses tabs but records use spaces, so columns may not line up perfectly — that's only cosmetic.
- Inserting before Create works too: `>>` creates the file if it doesn't exist.

**Viva**
- **Q: Why not `grep -v "^$name " $file > $file`?**
  A: The shell processes `> $file` *before* `grep` runs, so the file is emptied first and grep reads nothing. Result: all data lost. Hence `temp.txt` + `mv`.
- **Q: Difference between `>` and `>>`?** `>` truncates then writes; `>>` appends.
- **Q: What does `;;` do?** Ends a `case` branch (no fall-through).
- **Q: What is `$?`?** The exit status of the last command. `grep -q` sets it to 0 when found.
- **Q: What is `/dev/null`?** A special device file that discards anything written to it.
- **Q: How do you make a script executable?** `chmod +x file.sh`, then run `./file.sh`.
- **Q: Difference between `"$var"` and `'$var'`?** Double quotes expand variables; single quotes print `$var` literally.

---
---

# Assignment 2 — Process Management: fork, wait, Zombie, Orphan

**File:** `2_process.c`

## A. Aim

1. Read an array of integers.
2. Use `fork()` to create a child process. **Both** parent and child sort the array (bubble sort) and print it — showing that each process has its **own copy** of the data.
3. Parent uses `wait()` so it does not finish before the child.
4. Demonstrate a **Zombie** process (child dead, parent hasn't collected it yet).
5. Demonstrate an **Orphan** process (parent dead, child still running → adopted by `init`/`systemd`).

## B. Theory

### B.1 Process Control Block (PCB)
The kernel keeps a PCB (in Linux: `task_struct`) for every process, containing:
PID, parent PID (PPID), state, CPU registers & program counter, memory info, open-file table, scheduling info, **exit status**.

### B.2 `fork()` — creating a process
```
Before fork:              After fork:
┌────────────────┐        ┌────────────────┐      ┌────────────────┐
│ Parent PID=100 │        │ Parent PID=100 │      │ Child  PID=101 │
│ arr = [3,1,2]  │  ──►   │ arr = [3,1,2]  │      │ arr = [3,1,2]  │
│ pc → fork()    │        │ fork() → 101   │      │ fork() → 0     │
└────────────────┘        └────────────────┘      └────────────────┘
```
- Called **once**, returns **twice** (once in each process).
- Return value: **> 0** in parent (= child's PID), **0** in child, **−1** on failure (no child created).
- The child is an almost exact copy: same code, same variable values, same open files, *same program counter* (both continue from the line after `fork`).
- Memory is **not shared** afterwards. A change in one process is invisible to the other (Copy-On-Write).
- Execution order after `fork` is decided by the **scheduler** → output can interleave differently every run.

### B.3 `wait()` — waiting for a child
- `wait(NULL)` blocks the caller until **any one** child terminates, then removes the child's PCB from the process table (**reaping**).
- Passing `NULL` means "I don't care about the exit status". Passing `&status` would let you read it with `WEXITSTATUS(status)`.
- Returns the PID of the child that ended, or −1 if there are no children.

### B.4 `exit()` — terminating a process
- `exit(0)` flushes stdio buffers, closes files, and terminates the process. `0` = success.
- The process becomes a **zombie** until its parent calls `wait()`.

### B.5 Zombie process
- **Definition:** a process that has **finished execution** but still has an entry in the process table because the parent hasn't read its exit status yet.
- State shown by `ps` as **`Z`** / `<defunct>`.
- Holds no memory or CPU — only a PID and exit status. But too many zombies can exhaust the PID table.
- **Fix:** parent calls `wait()`/`waitpid()`, or parent dies (then `init` adopts and reaps it).

### B.6 Orphan process
- **Definition:** a process whose **parent has terminated** while it is still running.
- The kernel **re-parents** it to `init` (PID 1) — or on modern systems to `systemd`/a "subreaper" process. So `getppid()` changes from the old parent PID to 1 (or the subreaper's PID).
- Orphans are **not harmful**: init will `wait()` for them when they finish.

| | Zombie | Orphan |
|---|---|---|
| Who is dead? | the **child** | the **parent** |
| Who is alive? | the parent | the child |
| Running code? | No | Yes |
| Harmful? | Yes (wastes PID slot) | No |
| Cleaned by | parent's `wait()` | `init` adopts and reaps |

### B.7 Bubble sort (used in `sort`)
Repeatedly compare neighbours and swap if out of order. After pass *i*, the largest *i+1* elements are in their final place at the end. Time **O(n²)**, space **O(1)**.

## C. Functions & System Calls

| Function | Header | Prototype | Returns | Used for |
|---|---|---|---|---|
| `fork` | `<unistd.h>` | `pid_t fork(void);` | child PID / 0 / −1 | Create child |
| `getpid` | `<unistd.h>` | `pid_t getpid(void);` | own PID | Print identity |
| `getppid` | `<unistd.h>` | `pid_t getppid(void);` | parent's PID | Show parent / adoption |
| `sleep` | `<unistd.h>` | `unsigned sleep(unsigned sec);` | 0 when done | Create timing for zombie/orphan |
| `wait` | `<sys/wait.h>` | `pid_t wait(int *status);` | dead child's PID | Block + reap child |
| `exit` | `<stdlib.h>` | `void exit(int status);` | never returns | End process |
| `printf`/`scanf` | `<stdio.h>` | — | — | I/O |
| `sort` | user-defined | `void sort(int arr[], int n)` | — | Bubble sort |
| `print` | user-defined | `void print(int arr[], int n)` | — | Print array |

`pid_t` is just a signed integer type used for process IDs.

## D. Execution Flow (Call Chain)

**Function call chain:**
```
main()
 ├─ scanf → n, arr[]
 ├─ fork()  ═════════════════ splits into 2 processes ═════════════════
 │
 │  CHILD (pid == 0)                      PARENT (pid > 0)
 │  ├─ printf(getpid, getppid)            ├─ printf(getpid, pid)
 │  ├─ sort(arr,n) ──► returns            ├─ sort(arr,n) ──► returns
 │  ├─ print(arr,n) ─► returns            ├─ print(arr,n) ─► returns
 │  └─ exit(0)  ✖ child ends ───────────► ├─ wait(NULL)   ◄── blocks until child ends, reaps it
 │                                        ├─ printf("Child finished")
 │                                        │
 │                                        ├─ ZOMBIE DEMO: fork() ════════════════╗
 │                                        │    parent (z>0)               child (z==0)
 │                                        │    ├─ printf("...ZOMBIE")      ├─ printf("exiting")
 │                                        │    ├─ sleep(2) ◄─ child is     └─ exit(0) ✖
 │                                        │    │             ZOMBIE here
 │                                        │    ├─ wait(NULL) → reaped
 │                                        │    └─ printf("Reaped")
 │                                        │
 │                                        └─ ORPHAN DEMO: fork() ════════════════╗
 │                                             parent (o>0)               child (o==0)
 │                                             ├─ printf("ORPHAN")        ├─ sleep(2) … parent dies meanwhile
 │                                             └─ exit(0) ✖               ├─ printf(getppid → 1 / subreaper)
 │                                                                        └─ exit(0) ✖
```

**Short chain to memorise:**
`main → fork → {child: sort → print → exit} ‖ {parent: sort → print → wait → fork(zombie) → sleep → wait → fork(orphan) → exit}`

**Process tree over time:**
```
t=0      P(100)
t=1      P(100) ── C1(101)                 sort children
t=2      P(100)                            C1 reaped by wait
t=3      P(100) ── Z(102) <defunct>        zombie for 2 seconds
t=5      P(100)                            Z reaped by wait
t=6      P(100) ── O(103)
t=6.1    init(1) ── O(103)                 P exited → O adopted
t=8      O prints PPID=1, exits; init reaps it
```

## E. Line-by-Line Explanation

| Line | Code | Explanation |
|---|---|---|
| 1 | `#include <stdio.h>` | `printf`, `scanf`. |
| 2 | `#include <stdlib.h>` | `exit`. |
| 3 | `#include <unistd.h>` | POSIX calls: `fork`, `getpid`, `getppid`, `sleep`. |
| 4 | `#include <sys/wait.h>` | `wait`. |
| 6 | `void sort(int arr[], int n) {` | Bubble-sort function. `arr` is passed as a pointer, so it sorts the caller's array in place. |
| 7 | `for (int i = 0; i < n - 1; i++)` | Outer loop: `n-1` passes. |
| 8 | `for (int j = 0; j < n - i - 1; j++)` | Inner loop: compare neighbours; last `i` elements are already sorted, so stop earlier each pass. |
| 9 | `if (arr[j] > arr[j + 1]) {` | Out of order? |
| 10–12 | `temp = arr[j]; arr[j] = arr[j+1]; arr[j+1] = temp;` | Swap the two elements using a temporary variable. |
| 16 | `void print(int arr[], int n) {` | Helper to print an array. |
| 17 | `for (...) printf("%d ", arr[i]);` | Print each element followed by a space. |
| 18 | `printf("\n");` | End the line. |
| 21 | `int main() {` | Program starts here. |
| 22–24 | `int n; printf(...); scanf("%d", &n);` | Ask for and read the array size. `&n` = address where scanf stores the value. |
| 25 | `int arr[n];` | Variable-Length Array (C99) of size `n`, on the stack. |
| 26–27 | `printf(...); for (...) scanf("%d", &arr[i]);` | Read `n` integers. |
| 30 | `pid_t pid = fork();` | **Create the child.** From here on two processes run this code. Parent gets child's PID, child gets 0. |
| 32 | `if (pid == 0) {` | Only the **child** enters this block. |
| 34 | `printf("[Child] PID…PPID…", getpid(), getppid());` | Child prints its own PID and its parent's PID (= parent's `getpid()`). |
| 35 | `sort(arr, n);` | Child sorts **its own copy** of `arr`. |
| 36–37 | `printf(...); print(arr, n);` | Show child's sorted array. |
| 38 | `exit(0);` | Child terminates. It now waits as a zombie until the parent calls `wait`. Without `exit`, the child would continue past the `if` and run `return 0` — fine here, but in larger programs a child falling through into parent code is a classic bug. |
| 39 | `} else {` | **Parent** branch (`pid > 0`). (We don't check `-1` to keep code short.) |
| 41 | `printf("[Parent] PID…Child PID…", getpid(), pid);` | Parent shows its PID and the child's PID returned by `fork`. |
| 42–44 | `sort(arr, n); printf(...); print(arr, n);` | Parent sorts its **own** copy — proves the two processes have separate memory. |
| 45 | `wait(NULL);` | Parent **blocks** until the child exits, then reaps it. Prevents a lingering zombie. |
| 46 | `printf("[Parent] Child finished.\n");` | Runs only after the child is gone. |
| 49 | `printf("--- Demonstrating Zombie State ---")` | Section heading. |
| 50 | `pid_t z = fork();` | Second child, used for the zombie demo. |
| 51–53 | `if (z == 0) { printf(...); exit(0); }` | Child prints and **dies immediately**. |
| 54–55 | `else { printf("Sleeping 2s without wait()…ZOMBIE") ` | Parent announces it won't collect the child yet. |
| 56 | `sleep(2);` | **During these 2 seconds the child is a ZOMBIE** (dead, but still in the process table). Run `ps -el \| grep Z` in another terminal to see `<defunct>`. |
| 57 | `wait(NULL);` | Parent reaps the zombie → entry removed. |
| 58 | `printf("Reaped zombie child.")` | Confirmation. |
| 62 | `printf("--- Demonstrating Orphan State ---")` | Section heading. |
| 63 | `pid_t o = fork();` | Third child, for the orphan demo. |
| 64–65 | `if (o == 0) { sleep(2);` | Child sleeps, giving the parent time to die first. |
| 66 | `printf("[Orphan Child]…", getpid(), getppid());` | After waking, its original parent is gone → `getppid()` now shows **1** (init) or the PID of `systemd --user`/a subreaper. |
| 67 | `exit(0);` | Orphan ends; its new parent (init) reaps it. |
| 68–70 | `} else { printf("Exiting now…ORPHAN"); exit(0); }` | Parent exits **without** waiting → child becomes an orphan. |
| 73 | `return 0;` | Never actually reached (every path calls `exit`), kept for correctness. |

## F. Dry Run

**Input:** `n = 5`, array `4 1 3 5 2`. Assume parent PID = 1000.

```
Enter number of elements: 5
Enter 5 integers: 4 1 3 5 2

[Parent] PID: 1000, Child PID: 1001
[Parent] Sorted array: 1 2 3 4 5

[Child] PID: 1001, PPID: 1000
[Child] Sorted array: 1 2 3 4 5
[Parent] Child finished.

--- Demonstrating Zombie State ---
[Parent] Sleeping 2s without wait(). Child 1002 is ZOMBIE.
[Zombie Child] PID 1002 exiting now...
[Parent] Reaped zombie child.            ← printed after 2 s

--- Demonstrating Orphan State ---
[Parent] Exiting now. Child 1003 becomes ORPHAN.
$                                         ← shell prompt returns (parent is dead)
[Orphan Child] PID: 1003, Adopted by Parent PID: 1   ← 2 s later
```

**Bubble-sort trace on `[4,1,3,5,2]`:**

| Pass i | Array after pass | Comment |
|---|---|---|
| 0 | `1 3 4 2 5` | 5 bubbled to the end |
| 1 | `1 3 2 4 5` | 4 in place |
| 2 | `1 2 3 4 5` | 3 in place |
| 3 | `1 2 3 4 5` | done |

> Order of `[Parent]` and `[Child]` lines can change between runs — it depends on the scheduler.

## G. Compile & Run

```bash
gcc 2_process.c -o process
```
```bash
./process
```
In a second terminal during the zombie's 2-second window:
```bash
ps -el | grep -w Z
```

## H. Mistakes, Edge Cases & Viva

- **Output appears twice?** If you redirect output to a file, stdout becomes fully buffered and un-flushed text before `fork()` is copied into the child. On a terminal it's line-buffered, so it's fine. Fix: `fflush(stdout)` before `fork()`.
- **How many processes does `fork(); fork(); fork();` create?** 2³ = 8 total (7 new).
- **Q: Why does the child print the same unsorted input?** It doesn't; both sort their *own* copy. If the child sorted and the parent did not, the parent's array would still be unsorted — proof of separate memory.
- **Q: Difference between `wait()` and `waitpid()`?** `wait` waits for *any* child; `waitpid(pid, &st, opts)` waits for a *specific* child and supports `WNOHANG` (don't block).
- **Q: Can you kill a zombie with `kill -9`?** No, it's already dead. You must make the parent `wait()` or kill the parent.
- **Q: What is `exec()`?** Replaces the current process image with a new program (same PID). `fork` + `exec` is how the shell runs commands.
- **Q: What is PID 1?** `init`/`systemd`, the first user process, ancestor of all processes, adopts orphans.

---
---

# Assignment 3 — Thread Synchronization: Producer–Consumer

**File:** `3_producer_consumer.c`

## A. Aim

Implement the **bounded-buffer producer–consumer problem** with POSIX threads:
- A **producer** thread puts items (10, 20, 30, 40, 50) into a shared buffer of size 5.
- A **consumer** thread takes items out.
- Use **counting semaphores** so the producer never writes into a full buffer and the consumer never reads from an empty buffer, and a **mutex** so they never touch the buffer at the same time.

## B. Theory

### B.1 Threads
- A thread is a "lightweight process": it runs inside a process and **shares** its global variables, heap and files.
- Creating/switching threads is cheaper than processes.
- Because threads share data, two threads changing the same variable at once can corrupt it → we need **synchronization**.

### B.2 Race condition — why we need synchronization
`in = in + 1` is really three machine steps: *load in → add 1 → store in*. If two threads interleave:
```
Thread A: load in (0)
Thread B: load in (0)
Thread A: store 1
Thread B: store 1      ← one update lost; in should be 2
```
The result depends on timing → **race condition**.

### B.3 Critical section problem
The **critical section** is the code that accesses shared data (here: `buffer`, `in`, `out`). A correct solution must guarantee:
1. **Mutual exclusion** — at most one thread in the critical section.
2. **Progress** — if nobody is inside, someone who wants to enter must be allowed soon.
3. **Bounded waiting** — no thread waits forever.

### B.4 Mutex
- A **mutual exclusion lock**. `lock()` — if free, take it; if taken, **block**. `unlock()` — release it.
- Has **ownership**: only the thread that locked it should unlock it.

### B.5 Semaphore (Dijkstra)
An integer `S` with two **atomic** operations:
```
wait(S)   / P / sem_wait :   while (S <= 0) block;   S = S - 1;
signal(S) / V / sem_post :   S = S + 1;   wake one blocked thread (if any)
```
- **Binary semaphore**: values 0/1 (like a lock).
- **Counting semaphore**: any non-negative value — counts available resources.

### B.6 The bounded-buffer solution
| Semaphore / lock | Initial | Meaning |
|---|---|---|
| `empty` | `SIZE` (5) | number of **free** slots |
| `full` | `0` | number of **filled** slots |
| `mutex` | unlocked | protects `buffer`, `in`, `out` |

```
Producer:                       Consumer:
  wait(empty)   // free slot?     wait(full)    // item available?
  lock(mutex)                     lock(mutex)
    put item at buffer[in]          take item from buffer[out]
    in = (in+1) % SIZE              out = (out+1) % SIZE
  unlock(mutex)                   unlock(mutex)
  post(full)    // one more item  post(empty)   // one more free slot
```
**Invariant:** `empty + full == SIZE` (whenever no thread is in the middle of an operation).

### B.7 Circular buffer
`in` = where the next item is written, `out` = where the next item is read. `% SIZE` wraps the index from 4 back to 0, so the array is reused like a ring:
```
index:   0    1    2    3    4
        [10] [20] [  ] [  ] [  ]
              ▲    ▲
             out   in
```

## C. Functions & System Calls

| Function | Header | Prototype | Purpose here |
|---|---|---|---|
| `pthread_create` | `<pthread.h>` | `int pthread_create(pthread_t *t, const pthread_attr_t *attr, void *(*fn)(void *), void *arg);` | Start a thread running `fn(arg)`. Returns 0 on success |
| `pthread_join` | `<pthread.h>` | `int pthread_join(pthread_t t, void **ret);` | Wait for thread `t` to finish (like `wait` for threads) |
| `pthread_mutex_init` | `<pthread.h>` | `int pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *attr);` | Initialise mutex (`NULL` = default attributes) |
| `pthread_mutex_lock` / `unlock` | `<pthread.h>` | `int pthread_mutex_lock(pthread_mutex_t *m);` | Enter / leave critical section |
| `pthread_mutex_destroy` | `<pthread.h>` | `int pthread_mutex_destroy(pthread_mutex_t *m);` | Free mutex resources |
| `sem_init` | `<semaphore.h>` | `int sem_init(sem_t *s, int pshared, unsigned value);` | Create semaphore with initial `value`. `pshared = 0` → shared between threads of this process |
| `sem_wait` | `<semaphore.h>` | `int sem_wait(sem_t *s);` | P(): decrement, block if 0 |
| `sem_post` | `<semaphore.h>` | `int sem_post(sem_t *s);` | V(): increment, wake a waiter |
| `sem_destroy` | `<semaphore.h>` | `int sem_destroy(sem_t *s);` | Free semaphore |
| `sleep` | `<unistd.h>` | `unsigned sleep(unsigned s);` | Slow down so the interleaving is visible |

**Thread function signature:** must be `void *fn(void *arg)` — takes one generic pointer and returns one generic pointer.

## D. Execution Flow (Call Chain)

```
main()  [main thread]
 ├─ pthread_mutex_init(&mutex)
 ├─ sem_init(&empty, 0, 5)        empty = 5
 ├─ sem_init(&full,  0, 0)        full  = 0
 ├─ pthread_create(&p, producer) ════════╗  new thread starts producer()
 ├─ pthread_create(&c, consumer) ════════╬══════╗ new thread starts consumer()
 ├─ pthread_join(p) … blocks …           ║      ║
 │                                       ▼      ▼
 │              producer()                      consumer()
 │              loop i = 1..5:                  loop i = 1..5:
 │               ├ sem_wait(&empty)              ├ sem_wait(&full)  ← blocks while full==0
 │               ├ mutex_lock                    ├ mutex_lock
 │               ├ buffer[in]=i*10; printf       ├ item=buffer[out]; printf
 │               ├ in=(in+1)%5                   ├ out=(out+1)%5
 │               ├ mutex_unlock                  ├ mutex_unlock
 │               ├ sem_post(&full) ──wakes──►    ├ sem_post(&empty) ──wakes producer if blocked
 │               └ sleep(1)                      └ sleep(1)
 │              return NULL ✖                   return NULL ✖
 ├─ pthread_join(p) returns
 ├─ pthread_join(c) returns
 ├─ pthread_mutex_destroy, sem_destroy ×2
 └─ return 0
```

**Short chain:** `main → init(mutex, empty=5, full=0) → create(producer) ‖ create(consumer) → join(p) → join(c) → destroy → return`

## E. Line-by-Line Explanation

| Line | Code | Explanation |
|---|---|---|
| 1 | `#include <stdio.h>` | `printf`. |
| 2 | `#include <pthread.h>` | Threads & mutex API. |
| 3 | `#include <semaphore.h>` | POSIX unnamed semaphores. |
| 4 | `#include <unistd.h>` | `sleep`. |
| 6 | `#define SIZE 5` | Buffer capacity (compile-time constant). |
| 8 | `int buffer[SIZE];` | **Shared** circular buffer — global, so both threads see it. |
| 9 | `int in = 0, out = 0;` | Next write index (producer) and next read index (consumer). |
| 11 | `pthread_mutex_t mutex;` | Lock protecting the buffer and indices. |
| 12 | `sem_t empty, full;` | Counting semaphores for free and filled slots. |
| 14 | `void *producer(void *arg) {` | Producer thread function (signature required by `pthread_create`). `arg` unused. |
| 15 | `for (int i = 1; i <= 5; i++) {` | Produce 5 items. |
| 16 | `sem_wait(&empty);` | Take one free slot. If `empty == 0` (buffer full) → **block** until consumer posts. |
| 17 | `pthread_mutex_lock(&mutex);` | **Enter critical section.** |
| 19 | `buffer[in] = i * 10;` | Produce item 10, 20, … and store it. |
| 20 | `printf("Producer produced %d at index %d\n", …);` | Log. |
| 21 | `in = (in + 1) % SIZE;` | Move write index forward, wrapping 4 → 0. |
| 23 | `pthread_mutex_unlock(&mutex);` | **Leave critical section.** |
| 24 | `sem_post(&full);` | One more item available → may wake the consumer. |
| 25 | `sleep(1);` | Pause *outside* the lock, so the other thread gets a turn and output is readable. |
| 27 | `return NULL;` | Thread finishes (return value not used). |
| 30 | `void *consumer(void *arg) {` | Consumer thread function. |
| 31 | `for (int i = 1; i <= 5; i++) {` | Consume 5 items. |
| 32 | `sem_wait(&full);` | Take one filled slot. If `full == 0` (buffer empty) → **block** until producer posts. |
| 33 | `pthread_mutex_lock(&mutex);` | Enter critical section. |
| 35 | `int item = buffer[out];` | Remove item. |
| 36 | `printf("Consumer consumed %d from index %d\n", …);` | Log. |
| 37 | `out = (out + 1) % SIZE;` | Advance read index with wrap-around. |
| 39 | `pthread_mutex_unlock(&mutex);` | Leave critical section. |
| 40 | `sem_post(&empty);` | One more free slot → may wake the producer. |
| 41 | `sleep(1);` | Pause outside the lock. |
| 43 | `return NULL;` | Thread ends. |
| 46 | `int main() {` | Main thread. |
| 47 | `pthread_t p, c;` | Thread IDs (handles). |
| 49 | `pthread_mutex_init(&mutex, NULL);` | Initialise lock with default attributes. |
| 50 | `sem_init(&empty, 0, SIZE);` | `empty = 5`: all slots free. `0` = shared among threads of this process (not across processes). |
| 51 | `sem_init(&full, 0, 0);` | `full = 0`: nothing to consume yet. |
| 53 | `pthread_create(&p, NULL, producer, NULL);` | Start producer thread. Args: id out-param, default attributes, function, argument. |
| 54 | `pthread_create(&c, NULL, consumer, NULL);` | Start consumer thread. Both now run **concurrently** with main. |
| 56 | `pthread_join(p, NULL);` | Main waits for producer to finish. |
| 57 | `pthread_join(c, NULL);` | Main waits for consumer. Without joins, `main` would return and kill both threads early. |
| 59–61 | `pthread_mutex_destroy(...); sem_destroy(...) ×2` | Clean up synchronization objects. |
| 63 | `return 0;` | End of program. |

## F. Dry Run

| Step | Event | `empty` | `full` | `in` | `out` | Buffer |
|---|---|---|---|---|---|---|
| 0 | Initial | 5 | 0 | 0 | 0 | `[ _ _ _ _ _ ]` |
| 1 | Consumer: `sem_wait(full)` → full=0 → **blocks** | 5 | 0 | 0 | 0 | `[ _ _ _ _ _ ]` |
| 2 | Producer: wait(empty)→4, put 10 at 0, post(full) | 4 | 1 | 1 | 0 | `[10 _ _ _ _ ]` |
| 3 | Consumer wakes: full→0, take 10 from 0, post(empty) | 5 | 0 | 1 | 1 | `[ _ _ _ _ _ ]` |
| 4 | Producer: put 20 at 1 | 4 | 1 | 2 | 1 | `[ _ 20 _ _ _ ]` |
| 5 | Consumer: take 20 from 1 | 5 | 0 | 2 | 2 | … |
| … | continues for 30, 40, 50 | | | | | |
| end | Both loops done | 5 | 0 | 0 | 0 | indices wrapped back to 0 |

**Output:**
```
Producer produced 10 at index 0
Consumer consumed 10 from index 0
Producer produced 20 at index 1
Consumer consumed 20 from index 1
Producer produced 30 at index 2
Consumer consumed 30 from index 2
Producer produced 40 at index 3
Consumer consumed 40 from index 3
Producer produced 50 at index 4
Consumer consumed 50 from index 4
```
(Exact interleaving can vary; but the consumer can **never** print before the matching produce.)

## G. Compile & Run

```bash
gcc 3_producer_consumer.c -o pc -pthread
```
```bash
./pc
```
`-pthread` links the POSIX thread library (required for `pthread_*` and `sem_*`).

## H. Mistakes, Edge Cases & Viva

- **Q: What if we lock the mutex *before* `sem_wait(&empty)`?**
  If the buffer is full, the producer sleeps on `empty` *while holding the mutex*. The consumer can't lock the mutex to remove an item → nobody posts `empty` → **deadlock**. Rule: **wait on the semaphore first, then lock.**
- **Q: Why both semaphores and a mutex?** Semaphores handle *counting/conditions* (full/empty); the mutex handles *mutual exclusion* on the buffer.
- **Q: Can a binary semaphore replace the mutex?** Yes functionally (`sem_init(&s,0,1)`), but a mutex has ownership and is clearer.
- **Q: Why `sleep` outside the critical section?** Sleeping while holding the lock would block the other thread for no reason.
- **Q: `pshared` argument?** `0` = threads of one process; non-zero = semaphore in shared memory, usable between processes.
- **Q: Difference between process and thread?** See §0.1: threads share memory, processes don't.
- **Q: What if there are multiple producers?** The same code works — the mutex protects `in`, and `empty` counts slots for all producers.

---
---

# Assignment 4 — Deadlock Avoidance: Dining Philosophers

**File:** `4_dining.c`

## A. Aim

Simulate **5 philosophers** sitting at a round table with **5 forks** (one between each pair). Each philosopher thinks, then needs **both** neighbouring forks to eat. Use **semaphores** so that:
- two philosophers never use the same fork at once (mutual exclusion), and
- the system can **never deadlock**.

## B. Theory

### B.1 The problem
```
              P0
         F0        F1
      P4              P1
         F4        F2
          P3   F3   P2

 Philosopher i needs: left fork = F[i], right fork = F[(i+1) % 5]
   P0: F0,F1   P1: F1,F2   P2: F2,F3   P3: F3,F4   P4: F4,F0
```
It models processes competing for a limited set of shared resources.

### B.2 What is deadlock?
A set of processes where **each is waiting for a resource held by another** in the set, so none can ever continue.

### B.3 Four necessary conditions (Coffman) — all must hold
| Condition | In the dining philosophers |
|---|---|
| **Mutual exclusion** | A fork can be held by only one philosopher |
| **Hold and wait** | A philosopher holds the left fork while waiting for the right |
| **No preemption** | You can't snatch a fork from someone |
| **Circular wait** | P0 waits for P1's fork, P1 for P2's, … P4 for P0's |

**Naive deadlock:** all 5 pick up their left fork at the same instant → every right fork is taken → all wait forever.

### B.4 Ways to handle deadlock
| Strategy | Idea |
|---|---|
| **Prevention** | Make one of the four conditions impossible |
| **Avoidance** | Check every request and only grant it if the state stays *safe* (Banker's — Assignment 5) |
| **Detection & recovery** | Let it happen, detect the cycle, kill/rollback |
| **Ignore** (Ostrich) | Assume it's rare (what most OSs do) |

### B.5 Our solution: the "room" (footman) semaphore
- A counting semaphore `room = 4` (`N − 1`). A philosopher must enter the room before touching forks.
- At most **4** philosophers compete for **5** forks. By the **pigeonhole principle** at least one of them gets **both** forks, eats, and releases them → progress always continues.
- This **breaks circular wait** (a cycle needs all 5).

Other known solutions (good viva answers):
1. **Asymmetric**: odd philosophers pick left first, even pick right first.
2. **Pick both or none** (check both forks inside a critical section).
3. **Resource ordering**: always pick the lower-numbered fork first.
4. **Monitor** solution with states THINKING/HUNGRY/EATING.

### B.6 Starvation vs Deadlock
- **Deadlock:** nobody progresses.
- **Starvation:** the system progresses but one particular thread may wait indefinitely. (In our program each philosopher eats only once, so starvation isn't an issue.)

## C. Functions & System Calls

| Function | Header | Purpose here |
|---|---|---|
| `sem_init(&s, 0, v)` | `<semaphore.h>` | `room = 4`, each `fork_s[i] = 1` (binary semaphore = fork free) |
| `sem_wait(&s)` | `<semaphore.h>` | Enter room / pick up a fork (blocks if unavailable) |
| `sem_post(&s)` | `<semaphore.h>` | Put a fork down / leave room |
| `sem_destroy(&s)` | `<semaphore.h>` | Clean up |
| `pthread_create(&t, NULL, fn, arg)` | `<pthread.h>` | One thread per philosopher, `arg` = pointer to its ID |
| `pthread_join(t, NULL)` | `<pthread.h>` | Main waits for all philosophers |
| `sleep(1)` | `<unistd.h>` | Simulate eating time |

## D. Execution Flow (Call Chain)

```
main()
 ├─ sem_init(&room, 0, 4)
 ├─ for i=0..4: sem_init(&fork_s[i], 0, 1)          all forks free
 ├─ for i=0..4: id[i]=i; pthread_create(&t[i], philosopher, &id[i])
 │        ╠══► philosopher(&id[0])
 │        ╠══► philosopher(&id[1])
 │        ╠══► philosopher(&id[2])     5 threads run concurrently
 │        ╠══► philosopher(&id[3])
 │        ╚══► philosopher(&id[4])
 ├─ for i=0..4: pthread_join(t[i])  … waits …
 ├─ sem_destroy(room), sem_destroy(fork_s[0..4])
 └─ return 0

philosopher(arg)          [each thread]
 ├─ i = *(int*)arg
 ├─ printf "thinking"
 ├─ sem_wait(&room)               ← 5th philosopher blocks here until someone leaves
 ├─ sem_wait(&fork_s[i])          ← left fork
 ├─ sem_wait(&fork_s[(i+1)%5])    ← right fork
 ├─ printf "eating"; sleep(1)
 ├─ sem_post(&fork_s[(i+1)%5])    ← put right fork
 ├─ sem_post(&fork_s[i])          ← put left fork
 ├─ sem_post(&room)               ← leave room, lets a waiting philosopher in
 ├─ printf "finished eating"
 └─ return NULL
```

**Short chain:** `main → init(room=4, forks=1) → create ×5 → philosopher: think → room↓ → left↓ → right↓ → eat → right↑ → left↑ → room↑ → main: join ×5 → destroy`

## E. Line-by-Line Explanation

| Line | Code | Explanation |
|---|---|---|
| 1–4 | `#include …` | stdio (`printf`), pthread, semaphore, unistd (`sleep`). |
| 6 | `sem_t fork_s[5];` | One **binary semaphore per fork**. Value 1 = on the table, 0 = in someone's hand. Named `fork_s` because `fork` is already a system-call name. |
| 7 | `sem_t room;` | Counting semaphore limiting how many philosophers may try to eat at once. |
| 9 | `void *philosopher(void *arg) {` | Thread function; each of the 5 threads runs this. |
| 10 | `int i = *(int *)arg;` | Cast the generic `void*` back to `int*` and read the philosopher number (0–4). |
| 12 | `printf("Philosopher %d is thinking\n", i);` | Thinking phase. |
| 13 | `sem_wait(&room);` | Enter the room. First 4 succeed (4→0); the 5th **blocks** until someone leaves. This is the deadlock-prevention step. |
| 15 | `sem_wait(&fork_s[i]);` | Pick up the **left** fork. Blocks if a neighbour is holding it. |
| 16 | `sem_wait(&fork_s[(i + 1) % 5]);` | Pick up the **right** fork. `% 5` makes philosopher 4's right fork = fork 0 (round table). |
| 18 | `printf("--> Philosopher %d is eating\n", i);` | Has both forks → critical section. |
| 19 | `sleep(1);` | Eating takes 1 second. |
| 21 | `sem_post(&fork_s[(i + 1) % 5]);` | Put down right fork (may wake right neighbour). |
| 22 | `sem_post(&fork_s[i]);` | Put down left fork (may wake left neighbour). |
| 24 | `sem_post(&room);` | Leave the room → the blocked philosopher (if any) may enter. |
| 25 | `printf("Philosopher %d finished eating\n", i);` | Log. |
| 26 | `return NULL;` | Thread ends. |
| 29 | `int main() {` | Main thread. |
| 30 | `pthread_t t[5];` | 5 thread handles. |
| 31 | `int id[5];` | Separate storage for each philosopher's number — **one variable per thread**. |
| 33 | `sem_init(&room, 0, 4);` | Room allows **4** (= N − 1). |
| 34 | `for (...) sem_init(&fork_s[i], 0, 1);` | Every fork starts free (value 1). |
| 36–39 | `id[i] = i; pthread_create(&t[i], NULL, philosopher, &id[i]);` | Create philosopher *i* and pass the **address of `id[i]`**. Passing `&i` would be a bug: `i` keeps changing in the loop, so threads might all read the same/wrong number. |
| 40 | `for (...) pthread_join(t[i], NULL);` | Wait for all philosophers to finish. |
| 42–43 | `sem_destroy(...)` | Clean up all 6 semaphores. |
| 45 | `return 0;` | Done. |

## F. Dry Run

Assume all threads start almost together and P0–P3 get into the room first.

| Time | Event | `room` | Forks held (F0..F4) |
|---|---|---|---|
| t0 | P0,P1,P2,P3 enter room; P4 **blocks** on `room` | 0 | — |
| t0 | P0 takes F0, F1 → **eats** | 0 | F0:P0 F1:P0 |
| t0 | P1 takes … F1 busy → **blocks** on F1 | 0 | |
| t0 | P2 takes F2, F3 → **eats** | 0 | F2:P2 F3:P2 |
| t0 | P3 takes … F3 busy → **blocks** on F3 | 0 | |
| t1 | P0 puts F1, F0, leaves room → `room` 1; P4 enters (room 0) | 0 | |
| t1 | P1 gets F1, then F2 (after P2 releases) → eats | | |
| t1 | P4 takes F4, F0 → eats | | |
| … | P3 eventually gets F3, F4 → eats | | |
| end | Everyone has eaten once; all threads joined | 4 | all free |

**Sample output** (order varies each run):
```
Philosopher 0 is thinking
Philosopher 1 is thinking
Philosopher 2 is thinking
Philosopher 3 is thinking
Philosopher 4 is thinking
--> Philosopher 0 is eating
--> Philosopher 2 is eating
Philosopher 0 finished eating
Philosopher 2 finished eating
--> Philosopher 4 is eating
--> Philosopher 1 is eating
Philosopher 4 finished eating
Philosopher 1 finished eating
--> Philosopher 3 is eating
Philosopher 3 finished eating
```
**Key observation:** two *neighbours* are never "eating" at the same time (they share a fork), but non-neighbours (e.g. 0 and 2) can eat together.

## G. Compile & Run

```bash
gcc 4_dining.c -o dining -pthread
```
```bash
./dining
```

## H. Mistakes, Edge Cases & Viva

- **Q: Why `room = 4` and not 5?** With 5, all could grab their left fork → circular wait → deadlock. With 4, at least one gets both forks.
- **Q: Which Coffman condition does this break?** Circular wait.
- **Q: Is this prevention or avoidance?** Prevention (structural rule). Banker's algorithm is avoidance.
- **Q: Why `&id[i]` instead of `&i`?** All threads would share the single loop variable `i`, which changes as the loop continues → wrong IDs.
- **Q: Why is fork named `fork_s`?** `fork` is the name of the process-creation system call in `<unistd.h>`.
- **Q: Order of releasing forks?** Doesn't matter for correctness; release order matters less than acquisition order.
- **Q: Difference between deadlock and livelock?** In livelock threads keep changing state (e.g. pick up and put down forks repeatedly) but still make no progress.

---
---

# Assignment 5 — Deadlock Avoidance: Banker's Algorithm

**File:** `5_bankers.c`

## A. Aim

Given, for `p` processes and `r` resource types:
- the **Allocation** matrix (what each process currently holds),
- the **Max** matrix (the most each process may ever need),
- the **Available** vector (free instances of each resource),

compute the **Need** matrix and run the **Banker's Safety Algorithm** to decide whether the system is in a **SAFE** state. If it is, print a **safe sequence**.

## B. Theory

### B.1 Deadlock avoidance
Unlike prevention (Assignment 4), avoidance doesn't forbid any of the 4 conditions. Instead, before granting resources the OS checks: *"if I grant this, can every process still finish in some order?"* If yes → grant; if no → make the process wait.

### B.2 Safe state and safe sequence
- A **safe sequence** is an order `<P_a, P_b, …>` such that each process can get its **remaining need** from (currently available + what all earlier processes in the sequence will release when they finish).
- **Safe state** = at least one safe sequence exists → deadlock **cannot** happen.
- **Unsafe state** = no safe sequence → deadlock **may** happen (not guaranteed).

```
┌──────────────────────────────┐
│          All states          │
│   ┌─────────────┐            │
│   │    Safe     │   Unsafe   │
│   └─────────────┘  ┌──────┐  │
│                    │Dead- │  │
│                    │lock  │  │
│                    └──────┘  │
└──────────────────────────────┘
Deadlock ⊂ Unsafe.  Safe ∩ Deadlock = ∅.
```

### B.3 Why "Banker's"?
A bank never lends out its cash in a way that leaves it unable to satisfy all its customers' maximum credit limits. Resources = cash, processes = customers, Max = credit limit.

### B.4 Data structures (n = processes, m = resource types)
| Name | Size | Meaning |
|---|---|---|
| `Available[m]` | vector | free instances of each resource |
| `Max[n][m]` | matrix | maximum demand of each process |
| `Allocation[n][m]` | matrix | currently held |
| `Need[n][m]` | matrix | **`Need = Max − Allocation`** (still may request) |

### B.5 Safety algorithm (textbook form)
```
1. Work = Available;   Finish[i] = false for all i
2. Find i such that  Finish[i] == false  AND  Need[i] <= Work   (for every resource)
      If no such i → go to step 4
3. Work = Work + Allocation[i];  Finish[i] = true;  append Pi to sequence;  go to step 2
4. If Finish[i] == true for all i → SAFE, else UNSAFE
```
Why `Work += Allocation[i]`? We *pretend* Pi gets its Need, runs to completion, and then **returns everything it holds**. Its Allocation goes back to the pool.

Complexity: **O(m × n²)**.

> Our program uses `avail` directly as `Work` and `done` as `Finish`.

### B.6 Resource-request algorithm (theory only — not in our code)
When Pi requests `Request[i]`:
1. If `Request > Need[i]` → error (exceeded its max claim).
2. If `Request > Available` → Pi must wait.
3. Pretend to allocate: `Available −= Request; Allocation[i] += Request; Need[i] −= Request`.
4. Run the safety algorithm. If safe → grant. If unsafe → roll back, Pi waits.

## C. Functions Used

| Function | Header | Purpose |
|---|---|---|
| `printf` | `<stdio.h>` | Prompts, Need matrix, result |
| `scanf` | `<stdio.h>` | Read p, r, matrices, vector |

The whole algorithm lives in `main()`; there are no other functions or system calls. The logic is the interesting part.

## D. Execution Flow

```
main()
 ├─ INPUT PHASE
 │   ├─ scanf p, r
 │   ├─ scanf alloc[p][r]
 │   ├─ scanf max[p][r]
 │   └─ scanf avail[r]
 │
 ├─ NEED PHASE
 │   └─ for each i, j:  need[i][j] = max[i][j] − alloc[i][j]  → print
 │
 ├─ SAFETY PHASE  (repeat p rounds)
 │   └─ for k = 0..p-1                       ← round
 │        └─ for i = 0..p-1                  ← scan every process
 │             └─ if !done[i]:
 │                  ├─ possible = (need[i][j] ≤ avail[j] for all j)
 │                  └─ if possible:
 │                       ├─ avail += alloc[i]          (Pi finishes, releases)
 │                       ├─ seq[count++] = i
 │                       └─ done[i] = 1
 │
 └─ OUTPUT PHASE
     ├─ count == p → "SAFE" + print seq
     └─ else       → "UNSAFE"
```

**Why `p` rounds?** Each round marks at least one more process done if the state is safe. In the worst case one per round → `p` rounds are always enough.

## E. Line-by-Line Explanation

| Line | Code | Explanation |
|---|---|---|
| 1 | `#include <stdio.h>` | `printf` / `scanf`. |
| 3 | `int main() {` | Entire program is in `main`. |
| 4 | `int p, r, alloc[10][10], max[10][10], need[10][10], avail[10];` | `p` processes, `r` resource types; fixed arrays support up to 10×10. |
| 5 | `int done[10] = {0}, seq[10], count = 0;` | `done[i]` = has Pi "finished" in the simulation (all start 0 = false). `seq` stores the safe sequence; `count` = how many are in it. |
| 7–8 | `printf(...); scanf("%d %d", &p, &r);` | Read sizes. |
| 10–12 | `for i, for j: scanf(&alloc[i][j])` | Read Allocation row by row (row = process, column = resource). |
| 14–16 | `for i, for j: scanf(&max[i][j])` | Read Max matrix. |
| 18–19 | `for j: scanf(&avail[j])` | Read Available vector. |
| 22 | `printf("\nNeed Matrix:\n");` | Heading. |
| 23–24 | `for i … for j …` | Visit every cell. |
| 25 | `need[i][j] = max[i][j] - alloc[i][j];` | **Need = Max − Allocation.** |
| 26 | `printf("%d ", need[i][j]);` | Print one cell. |
| 28 | `printf("\n");` | New row. |
| 32 | `for (int k = 0; k < p; k++) {` | Up to `p` rounds of scanning. |
| 33 | `for (int i = 0; i < p; i++) {` | Look at each process in order P0, P1, …. |
| 34 | `if (!done[i]) {` | Skip processes already in the sequence. |
| 35 | `int possible = 1;` | Assume Pi can finish… |
| 36–41 | `for j: if (need[i][j] > avail[j]) { possible = 0; break; }` | …unless some resource it still needs is more than what's available. `break` stops checking early. |
| 42 | `if (possible) {` | Pi can get everything it needs → it can run to completion. |
| 43 | `for j: avail[j] += alloc[i][j];` | Pi finishes and **releases** its allocation → more resources become free for others. |
| 44 | `seq[count++] = i;` | Add Pi to the safe sequence, then increment count. |
| 45 | `done[i] = 1;` | Mark as finished. |
| 51 | `if (count == p) {` | Every process could finish → SAFE. |
| 52–54 | `printf(... "P%d " ...)` | Print the sequence like `< P1 P3 P4 P0 P2 >`. |
| 55–56 | `else printf("UNSAFE")` | At least one process could never get its need. |
| 59 | `return 0;` | End. |

## F. Dry Run (classic textbook example)

**Input:** `p = 5`, `r = 3` (resources A B C)

| Process | Allocation (A B C) | Max (A B C) | **Need = Max − Alloc** |
|---|---|---|---|
| P0 | 0 1 0 | 7 5 3 | 7 4 3 |
| P1 | 2 0 0 | 3 2 2 | 1 2 2 |
| P2 | 3 0 2 | 9 0 2 | 6 0 0 |
| P3 | 2 1 1 | 2 2 2 | 0 1 1 |
| P4 | 0 0 2 | 4 3 3 | 4 3 1 |

Available = `3 3 2`

**Safety algorithm trace:**

| Round k | i | Need[i] | avail before | Need ≤ avail? | avail after | seq |
|---|---|---|---|---|---|---|
| 0 | P0 | 7 4 3 | 3 3 2 | ✗ (7>3) | 3 3 2 | – |
| 0 | P1 | 1 2 2 | 3 3 2 | ✓ | 3+2, 3+0, 2+0 = **5 3 2** | P1 |
| 0 | P2 | 6 0 0 | 5 3 2 | ✗ (6>5) | 5 3 2 | P1 |
| 0 | P3 | 0 1 1 | 5 3 2 | ✓ | **7 4 3** | P1 P3 |
| 0 | P4 | 4 3 1 | 7 4 3 | ✓ | **7 4 5** | P1 P3 P4 |
| 1 | P0 | 7 4 3 | 7 4 5 | ✓ | **7 5 5** | P1 P3 P4 P0 |
| 1 | P2 | 6 0 0 | 7 5 5 | ✓ | **10 5 7** | P1 P3 P4 P0 P2 |
| 2–4 | — | all done, nothing changes | | | | |

`count = 5 = p` → **SAFE**. Final `avail = 10 5 7` = total resources in the system (a good self-check: Available + sum of all Allocation).

**Exact output:**
```
Need Matrix:
7 4 3
1 2 2
6 0 0
0 1 1
4 3 1

System is in SAFE State.
Safe Sequence: < P1 P3 P4 P0 P2 >
```

**Unsafe example:** change Available to `0 0 0`. No process has Need ≤ (0,0,0) except one with Need all zeros → none here → `count = 0` → `UNSAFE`.

## G. Compile & Run

```bash
gcc 5_bankers.c -o bankers
```
```bash
./bankers
```
Input for the example above (paste all at once):
```
5 3
0 1 0
2 0 0
3 0 2
2 1 1
0 0 2
7 5 3
3 2 2
9 0 2
2 2 2
4 3 3
3 3 2
```

## H. Mistakes, Edge Cases & Viva

- **Is the safe sequence unique?** No. Our code gives the one found by scanning P0→P(p-1) each round. Others may exist (e.g. `P3 P1 P4 P0 P2`).
- **Unsafe ⇒ deadlock?** No. Unsafe means deadlock *may* occur. Safe guarantees it won't.
- **Limitations of Banker's:** needs the max demand in advance; number of processes and resources must be fixed; it's slow (O(m·n²) per request); processes must eventually release resources.
- **Q: Why add `alloc` (not `need`) to `avail`?** When Pi finishes it releases everything it holds — that's its Allocation. (Need was only borrowed temporarily during the simulation and returned too, so the net gain is Allocation.)
- **Q: Avoidance vs prevention?** Prevention removes a Coffman condition by design; avoidance checks the state dynamically before every grant.
- **Q: What if `max < alloc`?** Need becomes negative → invalid input; real implementations reject it.
- **Q: What's a resource-allocation graph?** A graph of processes and resources; a cycle means deadlock *if* each resource has a single instance. Banker's handles **multiple instances**.

---
---

# Assignment 6 — IPC: Full-Duplex Communication using FIFOs

**Files:** `6_fifo1.c` (process 1) and `6_fifo2.c` (process 2) — two **separate programs** run in two terminals.

## A. Aim

Two **unrelated** processes talk to each other in **both directions** through named pipes (FIFOs):
1. **Process 1** reads sentences from the user (until `#`) and sends them to Process 2 through `fifo1`.
2. **Process 2** counts the **characters, words and lines**, writes the result into a text file `result.txt`, reads that file back, and sends its contents to Process 1 through `fifo2`.
3. **Process 1** prints the result.

```
            fifo1  (sentences)
  ┌───────────┐ ─────────────────────────► ┌───────────┐
  │ Process 1 │                            │ Process 2 │──► result.txt
  │ 6_fifo1   │ ◄───────────────────────── │ 6_fifo2   │◄── (read back)
  └───────────┘   fifo2  (analysis text)    └───────────┘
```

## B. Theory

### B.1 Inter-Process Communication (IPC)
Processes have separate memory (Assignment 2), so they need kernel help to exchange data. IPC mechanisms:

| Mechanism | Between | Direction | Has a name in filesystem? |
|---|---|---|---|
| **Pipe** (`pipe()`) | Related processes (parent/child) | One-way per pipe | No |
| **FIFO / named pipe** (`mkfifo()`) | **Any** processes | One-way per FIFO | **Yes** |
| Message queue | Any | Both | Key-based |
| Shared memory | Any | Both (fastest) | Key-based |
| Socket | Any, even across machines | Both | Address-based |
| Signal | Any | Notification only | — |

### B.2 FIFO (named pipe)
- A **special file** in the filesystem (`ls -l` shows it as type `p`: `prw-rw-r-- fifo1`).
- Data written by one process is read by another in **First-In-First-Out** order.
- Data is **not stored on disk** — it goes through a kernel buffer. The file only acts as a *meeting point / name*.
- Once read, data is removed from the FIFO.

### B.3 Half-duplex vs full-duplex
- A single FIFO is **half-duplex** (data flows one way at a time).
- **Full-duplex** (both ways) needs **two FIFOs**: `fifo1` for P1→P2, `fifo2` for P2→P1.

### B.4 Blocking behaviour of `open()` on a FIFO — very important
- `open(fifo, O_WRONLY)` **blocks until some process opens it for reading**.
- `open(fifo, O_RDONLY)` **blocks until some process opens it for writing**.
- `read()` blocks until data is available (returns 0 when all writers have closed = EOF).

That's how the two programs **synchronize automatically**: whichever starts first just waits for the other.

### B.5 Permissions `0666`
Octal: owner `rw-` (6), group `rw-` (6), others `rw-` (6). The real permission is `0666 & ~umask` (typically `0664`).

### B.6 Low-level I/O vs stdio
| Low-level (system calls) | stdio (library, buffered) |
|---|---|
| `open`, `read`, `write`, `close` | `fopen`, `fread`, `fprintf`, `fclose` |
| works with **int fd** | works with **FILE \*** |
| used here for the FIFOs | used here for `result.txt` |

### B.7 Counting words, characters, lines
- **Characters:** every byte counts (including spaces and `\n`).
- **Lines:** count `\n`, plus 1 if the text doesn't end with `\n` (the last unfinished line).
- **Words:** count the positions where a word **starts** = a non-whitespace character that is either the first character or comes right after whitespace.

## C. Functions & System Calls

| Function | Header | Prototype | Returns | Used for |
|---|---|---|---|---|
| `mkfifo` | `<sys/stat.h>` | `int mkfifo(const char *path, mode_t mode);` | 0 / −1 (−1 with `EEXIST` if already present — harmless here) | Create the FIFO file |
| `open` | `<fcntl.h>` | `int open(const char *path, int flags);` | fd / −1 | Open FIFO with `O_RDONLY` or `O_WRONLY` |
| `write` | `<unistd.h>` | `ssize_t write(int fd, const void *buf, size_t n);` | bytes written | Send data |
| `read` | `<unistd.h>` | `ssize_t read(int fd, void *buf, size_t n);` | bytes read (0 = EOF) | Receive data |
| `close` | `<unistd.h>` | `int close(int fd);` | 0 | Release fd (reader sees EOF) |
| `strlen` | `<string.h>` | `size_t strlen(const char *s);` | length without `'\0'` | `+1` to send the terminator too |
| `scanf("%[^#]")` | `<stdio.h>` | — | — | Read **everything up to `#`**, including spaces and newlines |
| `fopen` / `fclose` | `<stdio.h>` | `FILE *fopen(const char *p, const char *mode);` | FILE* / NULL | Open `result.txt` (`"w"` = write/truncate, `"r"` = read) |
| `fprintf` | `<stdio.h>` | `int fprintf(FILE *f, const char *fmt, ...);` | chars written | Write counts into the file |
| `fread` | `<stdio.h>` | `size_t fread(void *buf, size_t size, size_t count, FILE *f);` | items read | Read the file back |

## D. Execution Flow (both programs, side by side)

```
TERMINAL 1: ./fifo1                                TERMINAL 2: ./fifo2
────────────────────────────                       ────────────────────────────
main()                                             main()
 ├ mkfifo("fifo1"), mkfifo("fifo2")                 ├ mkfifo("fifo1"), mkfifo("fifo2")  (EEXIST ok)
 ├ printf "Enter sentences"                         │
 ├ scanf("%[^#]", str)  ← user types … #            ├ open("fifo1", O_RDONLY) ─┐ blocks until a writer
 │                                                  │                          │
 ├ open("fifo1", O_WRONLY) ─── both opens meet ─────┼──────────────────────────┘ unblocks
 ├ write(fd1, str) ═══════════ fifo1 ══════════════►├ read(fd1, str)
 ├ close(fd1)                                       ├ close(fd1)
 │                                                  ├ count loop → c, w, l
 ├ open("fifo2", O_RDONLY) ─┐ blocks                ├ fopen("result.txt","w") → fprintf → fclose
 │                          │                       ├ fopen("result.txt","r") → fread → fclose
 │                          └── meets ──────────────├ open("fifo2", O_WRONLY)
 ├ read(fd2, res) ◄═══════════ fifo2 ═══════════════├ write(fd2, res)
 ├ close(fd2)                                       ├ close(fd2)
 ├ printf res                                       ├ printf "Analysis written…"
 └ return 0                                         └ return 0
```

**Short chains:**
- `fifo1: main → mkfifo×2 → scanf → open(fifo1,W) → write → close → open(fifo2,R) → read → close → printf`
- `fifo2: main → mkfifo×2 → open(fifo1,R) → read → close → count → fopen(w) → fprintf → fclose → fopen(r) → fread → fclose → open(fifo2,W) → write → close`

## E. Line-by-Line Explanation

### `6_fifo1.c` — the sender / displayer

| Line | Code | Explanation |
|---|---|---|
| 1 | `#include <stdio.h>` | `printf`, `scanf`. |
| 2 | `#include <string.h>` | `strlen`. |
| 3 | `#include <fcntl.h>` | `open` and flags `O_RDONLY`, `O_WRONLY`. |
| 4 | `#include <unistd.h>` | `read`, `write`, `close`. |
| 5 | `#include <sys/stat.h>` | `mkfifo`. |
| 8 | `char str[500], res[500];` | `str` = sentences to send; `res` = result received. |
| 9 | `int fd1, fd2;` | File descriptors for the two FIFOs. |
| 11 | `mkfifo("fifo1", 0666);` | Create FIFO `fifo1` in the current directory (if it already exists, returns −1 and we just ignore it). |
| 12 | `mkfifo("fifo2", 0666);` | Create FIFO `fifo2`. Both programs create both so it doesn't matter which runs first. |
| 14 | `printf("Enter sentences (end with #):\n");` | Prompt. |
| 15 | `scanf("%[^#]", str);` | **Scanset**: read every character that is *not* `#` — including spaces and newlines — so multiple lines can be entered. Stops at `#` (which is left unread). |
| 18 | `fd1 = open("fifo1", O_WRONLY);` | Open `fifo1` for writing. **Blocks until Process 2 opens it for reading.** |
| 19 | `write(fd1, str, strlen(str) + 1);` | Send the text **plus `'\0'`** (`+1`) so the receiver gets a proper C string. |
| 20 | `close(fd1);` | Done writing; reader will see EOF afterwards. |
| 23 | `fd2 = open("fifo2", O_RDONLY);` | Open `fifo2` for reading. **Blocks until Process 2 opens it for writing** (after it finishes counting). |
| 24 | `read(fd2, res, sizeof(res));` | Wait for and receive the analysis text (up to 500 bytes). |
| 25 | `close(fd2);` | Close the FIFO. |
| 27 | `printf("\nResult from second process:\n%s\n", res);` | Display the result. |
| 28 | `return 0;` | End. |

### `6_fifo2.c` — the processor

| Line | Code | Explanation |
|---|---|---|
| 1–5 | `#include …` | Same headers as fifo1 (`stdio` also gives `FILE`, `fopen`, `fprintf`, `fread`). |
| 8 | `char str[500], res[500];` | `str` = received sentences; `res` = analysis text to send back. |
| 9 | `int fd1, fd2, c = 0, w = 0, l = 0;` | FIFO fds; counters for **c**haracters, **w**ords, **l**ines. |
| 10 | `FILE *fp;` | Handle for `result.txt`. |
| 12–13 | `mkfifo(...)` ×2 | Create FIFOs if not already present. |
| 16 | `fd1 = open("fifo1", O_RDONLY);` | Open for reading — **blocks until Process 1 opens for writing**. |
| 17 | `read(fd1, str, sizeof(str));` | Receive the sentences (includes the `'\0'` that P1 sent). |
| 18 | `close(fd1);` | Close. |
| 21 | `for (int i = 0; str[i] != '\0'; i++) {` | Walk through each character until the string terminator. |
| 22 | `c++;` | Every character counts. |
| 23 | `if (str[i] == '\n') l++;` | Each newline ends a line. |
| 24–26 | `if (str[i] is not space/newline/tab && (i == 0 \|\| str[i-1] is space/newline/tab)) w++;` | **Start of a word**: current char is a letter/symbol AND it's either the very first char or the previous char was whitespace. Counting starts (not ends) handles multiple spaces correctly. |
| 28 | `if (c > 0 && str[c - 1] != '\n') l++;` | If the text doesn't end with `\n`, the last line wasn't counted yet → add 1. (`c` = length, so `str[c-1]` = last char.) |
| 31 | `fp = fopen("result.txt", "w");` | Create/truncate `result.txt` for writing. |
| 32 | `fprintf(fp, "Characters: %d\nWords: %d\nLines: %d\n", c, w, l);` | Write the analysis into the file. |
| 33 | `fclose(fp);` | Close → flushes the data to disk. |
| 36 | `fp = fopen("result.txt", "r");` | Reopen the same file for reading (the assignment asks to send the *file contents* back). |
| 37 | `int n = fread(res, 1, sizeof(res) - 1, fp);` | Read up to 499 bytes; `n` = bytes actually read. |
| 38 | `res[n] = '\0';` | `fread` doesn't add a terminator → add it so `res` is a valid string. |
| 39 | `fclose(fp);` | Close file. |
| 42 | `fd2 = open("fifo2", O_WRONLY);` | Open `fifo2` for writing — unblocks Process 1, which is waiting in its `open(…O_RDONLY)`. |
| 43 | `write(fd2, res, strlen(res) + 1);` | Send the file contents including `'\0'`. |
| 44 | `close(fd2);` | Close. |
| 46 | `printf("Analysis written to result.txt and sent back via fifo2.\n");` | Status message on terminal 2. |
| 47 | `return 0;` | End. |

## F. Dry Run

**Terminal 1 input:**
```
Hello world
OS lab#
```
`str` = `"Hello world\nOS lab"` (the `#` is not included).

**Counting in Process 2:**

| Index | Char | c | Word start? | w | Newline? | l |
|---|---|---|---|---|---|---|
| 0 | `H` | 1 | i==0 ✓ | 1 | | 0 |
| 1–4 | `ello` | 5 | | 1 | | 0 |
| 5 | ` ` | 6 | | 1 | | 0 |
| 6 | `w` | 7 | prev is space ✓ | 2 | | 0 |
| 7–10 | `orld` | 11 | | 2 | | 0 |
| 11 | `\n` | 12 | | 2 | ✓ | 1 |
| 12 | `O` | 13 | prev is `\n` ✓ | 3 | | 1 |
| 13 | `S` | 14 | | 3 | | 1 |
| 14 | ` ` | 15 | | 3 | | 1 |
| 15 | `l` | 16 | ✓ | 4 | | 1 |
| 16–17 | `ab` | 18 | | 4 | | 1 |
| after loop | last char `b` ≠ `\n` → l++ | 18 | | 4 | | **2** |

**`result.txt`:**
```
Characters: 18
Words: 4
Lines: 2
```

**Terminal 1 output:**
```
Result from second process:
Characters: 18
Words: 4
Lines: 2
```
**Terminal 2 output:** `Analysis written to result.txt and sent back via fifo2.`

## G. Compile & Run

```bash
gcc 6_fifo1.c -o fifo1
```
```bash
gcc 6_fifo2.c -o fifo2
```
Terminal 1:
```bash
./fifo1
```
Terminal 2 (same directory):
```bash
./fifo2
```
Check the FIFO files: `ls -l fifo1 fifo2` → first character `p` means pipe.

## H. Mistakes, Edge Cases & Viva

- **Both programs must run in the same directory** — the FIFO names are relative paths.
- **Input longer than 499 characters** would overflow `str` (no width limit in `%[^#]`). `%499[^#]` would be the safe version.
- **Q: Pipe vs FIFO?** Pipe: no name, only between related processes, disappears when closed. FIFO: has a filesystem name, any processes, persists until `unlink`/`rm`.
- **Q: Where is FIFO data stored?** In a kernel buffer, not on disk. The FIFO file has size 0.
- **Q: Why does the program "hang" if you run only one side?** `open()` on a FIFO blocks until the other end is opened.
- **Q: Why `strlen(str) + 1`?** To send the `'\0'` so the receiver can use it as a string directly.
- **Q: How to delete a FIFO?** `rm fifo1` or `unlink("fifo1")` in code.
- **Q: What does `O_NONBLOCK` change?** `open`/`read` return immediately instead of waiting (with error `ENXIO`/`EAGAIN` if the other side isn't ready).

---
---

# Assignment 7 — IPC: Full-Duplex Communication using Pipes

**File:** `7_pipes.c`

## A. Aim

Use **two unnamed pipes** between a **parent and child** process:
1. **Parent** reads a **file name** from the user and sends it to the child through **pipe 1**.
2. **Child** opens that file, reads its contents and sends them back through **pipe 2** (or sends `File not found!`).
3. **Parent** prints what it received.

```
           pipe p1 (file name)
  ┌────────┐ p1[1] ═══════════► p1[0] ┌────────┐
  │ PARENT │                          │ CHILD  │──fopen──► file on disk
  │        │ p2[0] ◄═══════════ p2[1] │        │
  └────────┘    pipe p2 (contents)    └────────┘
```

## B. Theory

### B.1 What is a pipe?
- A **unidirectional** byte channel in the kernel with two ends:
  - `fd[0]` → **read end**
  - `fd[1]` → **write end**
  (Memory trick: **0 = input like stdin, 1 = output like stdout**.)
- Data comes out in the same order it went in (FIFO). Kernel buffer is typically 64 KB on Linux.
- A pipe has **no name**, so only processes that inherit the fds (via `fork`) can use it → **related processes only**.

### B.2 How fork makes a pipe usable
```
After pipe(p1):              After fork():
 Parent                       Parent              Child
  p1[0] ─┐                     p1[0] ─┐  ┌─ p1[0]
  p1[1] ─┼─► [kernel buffer]   p1[1] ─┼──┼─ p1[1]   ──► same kernel pipe
                                       └──┘
```
Both processes now hold **both** ends. Each must **close the ends it doesn't use**.

### B.3 Why close unused ends?
- **EOF detection:** `read()` returns 0 (EOF) only when **all** write ends are closed. If the reader still holds its own copy of the write end, it will never see EOF and may block forever.
- **Clarity & resources:** frees file descriptors and makes the direction explicit.
- **SIGPIPE:** writing to a pipe with no open read end kills the writer with `SIGPIPE`.

### B.4 Full duplex needs two pipes
A single pipe is one-way. If both processes read and wrote the same pipe, one might read back its own data. So:
- `p1`: parent → child (parent writes `p1[1]`, child reads `p1[0]`)
- `p2`: child → parent (child writes `p2[1]`, parent reads `p2[0]`)

### B.5 Blocking rules
| Situation | Result |
|---|---|
| `read` on empty pipe, writer still open | blocks |
| `read` on empty pipe, all writers closed | returns 0 (EOF) |
| `write` to full pipe | blocks |
| `write` with all readers closed | `SIGPIPE` / error `EPIPE` |
| Writes ≤ `PIPE_BUF` (4096 bytes) | atomic (not mixed with other writes) |

## C. Functions & System Calls

| Function | Header | Prototype | Returns | Used for |
|---|---|---|---|---|
| `pipe` | `<unistd.h>` | `int pipe(int fd[2]);` | 0 / −1 | Create a pipe; fills `fd[0]` (read), `fd[1]` (write) |
| `fork` | `<unistd.h>` | `pid_t fork(void);` | child PID / 0 / −1 | Create the child that inherits the pipes |
| `close` | `<unistd.h>` | `int close(int fd);` | 0 | Close unused ends |
| `write` | `<unistd.h>` | `ssize_t write(int fd, const void *buf, size_t n);` | bytes | Send through pipe |
| `read` | `<unistd.h>` | `ssize_t read(int fd, void *buf, size_t n);` | bytes / 0 at EOF | Receive (blocks until data) |
| `wait` | `<sys/wait.h>` | `pid_t wait(int *st);` | child PID | Parent reaps child |
| `fopen` / `fread` / `fclose` | `<stdio.h>` | — | — | Child reads the requested file |
| `strlen` / `strcpy` | `<string.h>` | — | — | Lengths, error message |

## D. Execution Flow (Call Chain)

```
main()
 ├─ pipe(p1)      p1[0]=3, p1[1]=4   (typical fd numbers)
 ├─ pipe(p2)      p2[0]=5, p2[1]=6
 ├─ fork() ════════════════════════════════════════════════════
 │
 │  PARENT (fork() > 0)                    CHILD (fork() == 0)
 │  ├─ close(p1[0])  won't read p1         ├─ close(p1[1])  won't write p1
 │  ├─ close(p2[1])  won't write p2        ├─ close(p2[0])  won't read p2
 │  ├─ printf "Enter file name"            ├─ read(p1[0], fname) ── blocks until parent writes
 │  ├─ scanf fname                         │
 │  ├─ write(p1[1], fname) ═══ p1 ════════►├─ (unblocks, fname received)
 │  ├─ read(p2[0], buf) ── blocks          ├─ fopen(fname, "r")
 │  │                                      │    ├─ NULL → strcpy(buf,"File not found!")
 │  │                                      │    └─ ok   → fread(buf) → buf[n]='\0' → fclose
 │  ├─ (unblocks) ◄════════════ p2 ════════├─ write(p2[1], buf)
 │  ├─ printf buf                          └─ return 0 → child exits ✖
 │  ├─ wait(NULL) ── reaps child
 │  └─ return 0
```

**Short chain:** `main → pipe(p1) → pipe(p2) → fork → {parent: close×2 → scanf → write(p1) → read(p2) → printf → wait} ‖ {child: close×2 → read(p1) → fopen → fread → fclose → write(p2) → exit}`

## E. Line-by-Line Explanation

| Line | Code | Explanation |
|---|---|---|
| 1 | `#include <stdio.h>` | `printf`, `scanf`, `FILE`, `fopen`, `fread`. |
| 2 | `#include <string.h>` | `strlen`, `strcpy`. |
| 3 | `#include <unistd.h>` | `pipe`, `fork`, `read`, `write`, `close`. |
| 4 | `#include <sys/wait.h>` | `wait`. |
| 7 | `int p1[2], p2[2], n;` | Two pipes (each needs 2 fds) and `n` for bytes read from the file. |
| 8 | `char fname[100], buf[1000];` | File name (max 99 chars) and file content buffer (max 999 chars). |
| 9 | `FILE *fp;` | File handle used by the child. |
| 11 | `pipe(p1);` | Create pipe 1 **before** fork so both processes inherit it. Direction: parent → child. |
| 12 | `pipe(p2);` | Create pipe 2. Direction: child → parent. |
| 14 | `if (fork() > 0) {` | Fork and branch: positive return → this is the **parent**. |
| 16 | `close(p1[0]);` | Parent doesn't read from p1. |
| 17 | `close(p2[1]);` | Parent doesn't write to p2. |
| 19–20 | `printf("Enter file name: "); scanf("%s", fname);` | Read the file name (one word, no spaces). |
| 21 | `write(p1[1], fname, strlen(fname) + 1);` | Send the name **with `'\0'`** to the child. |
| 23 | `read(p2[0], buf, sizeof(buf));` | **Block** until the child sends the contents back. |
| 24 | `printf("\nFile contents received by parent:\n%s\n", buf);` | Print what came back. |
| 25 | `wait(NULL);` | Reap the child (avoid a zombie). |
| 26 | `} else {` | `fork()` returned 0 → **child**. (−1 error case not handled for brevity.) |
| 28 | `close(p1[1]);` | Child doesn't write to p1. |
| 29 | `close(p2[0]);` | Child doesn't read from p2. |
| 31 | `read(p1[0], fname, sizeof(fname));` | **Block** until the parent sends the name. |
| 33 | `fp = fopen(fname, "r");` | Try to open that file for reading. |
| 34–35 | `if (fp == NULL) strcpy(buf, "File not found!");` | File missing / no permission → prepare an error message instead. |
| 37 | `n = fread(buf, 1, sizeof(buf) - 1, fp);` | Read up to 999 bytes; leave room for the terminator. |
| 38 | `buf[n] = '\0';` | Make it a valid C string. |
| 39 | `fclose(fp);` | Close the file. |
| 41 | `write(p2[1], buf, strlen(buf) + 1);` | Send the contents (or error message) back to the parent. |
| 43 | `return 0;` | **Both** processes reach here: child exits (parent's `wait` returns), then parent exits. |

## F. Dry Run

Suppose `notes.txt` contains:
```
Operating Systems
Lab 7
```

| Step | Parent | Pipe p1 | Pipe p2 | Child |
|---|---|---|---|---|
| 1 | `pipe`, `pipe`, `fork` | empty | empty | created |
| 2 | closes p1[0], p2[1] | | | closes p1[1], p2[0] |
| 3 | prints prompt, waits for keyboard | | | blocked in `read(p1[0])` |
| 4 | user types `notes.txt`; `write(p1[1])` | `notes.txt\0` | | |
| 5 | blocked in `read(p2[0])` | → consumed | | reads `notes.txt` |
| 6 | | | | `fopen` OK, `fread` 24 bytes |
| 7 | | | `Operating Systems\nLab 7\n\0` | `write(p2[1])`, returns, exits |
| 8 | `read` returns, prints contents | | → consumed | (zombie briefly) |
| 9 | `wait(NULL)` reaps child, exits | | | gone |

**Output:**
```
Enter file name: notes.txt

File contents received by parent:
Operating Systems
Lab 7

```
**With a wrong name** (`abc.txt`):
```
Enter file name: abc.txt

File contents received by parent:
File not found!
```

## G. Compile & Run

```bash
gcc 7_pipes.c -o pipes
```
```bash
./pipes
```

## H. Mistakes, Edge Cases & Viva

- **Files bigger than 999 bytes** are truncated (buffer size). A loop of `read` calls would handle any size.
- **File name with spaces** won't work because `scanf("%s")` stops at whitespace.
- **Q: Which end is read, which is write?** `fd[0]` read, `fd[1]` write.
- **Q: Why create the pipes *before* `fork()`?** The child must inherit the same file descriptors; a pipe created after the fork exists only in one process.
- **Q: Can a pipe be used between unrelated processes?** No — use a FIFO (Assignment 6).
- **Q: What is `popen()`?** A library shortcut that creates a pipe, forks, and runs a shell command connected to it.
- **Q: How does `ls | wc -l` work?** The shell creates a pipe, forks twice, uses `dup2` to connect `ls`'s stdout to the write end and `wc`'s stdin to the read end, then `exec`s both.
- **Q: Half or full duplex?** One pipe = half duplex. Two pipes (our program) = full duplex.

---
---

# Assignment 8 — Disk Scheduling: SSTF, SCAN, C-LOOK

**File:** `8_disk.c`

## A. Aim

Given the **disk size** (number of cylinders), the **initial head position**, and a **queue of cylinder requests**, simulate three disk-scheduling algorithms and for each print:
- the order in which cylinders are visited, and
- the **total head movement** (total cylinders travelled).

Algorithms: **SSTF**, **SCAN** (elevator, towards higher cylinders first), **C-LOOK**.

## B. Theory

### B.1 Disk structure
```
        ┌───────────── platter (spins)
   ═════╪═════  track = a ring on one surface
        │       cylinder = the same track on all surfaces (numbered 0 … size-1)
   ─────┘       sector = a slice of a track
   ▲
   read/write head on an arm that moves in/out across cylinders
```

### B.2 Disk access time
**Access time = Seek time + Rotational latency + Transfer time**
- **Seek time:** move the arm to the right cylinder — the **largest** part.
- **Rotational latency:** wait for the sector to rotate under the head.
- **Transfer time:** actually read/write the data.

Disk scheduling tries to **minimise total seek distance** (head movement) → higher throughput and lower response time.

### B.3 The algorithms

| Algorithm | Rule | Pros | Cons |
|---|---|---|---|
| **FCFS** | Serve in arrival order | Fair, simple | Lots of back-and-forth movement |
| **SSTF** (Shortest Seek Time First) | Always go to the **closest** pending request | Much less movement than FCFS | **Starvation** of far requests; not optimal |
| **SCAN** (Elevator) | Move in one direction serving requests **until the disk end**, then reverse | No starvation, uniform-ish wait | Goes to the end even if no requests there |
| **C-SCAN** | Like SCAN, but after reaching the end **jump to cylinder 0** and continue in the same direction | More uniform waiting time | Long return seek |
| **LOOK** | Like SCAN but reverse at the **last request**, not the disk end | Less movement than SCAN | — |
| **C-LOOK** | Like C-SCAN but only go as far as the **last request**, then jump to the **lowest request** | Least wasted movement among the "elevator" family | Jump distance still costs time |

### B.4 Visual comparison (head = 50, requests = 16 24 43 82 140 170 190, disk 0–199)
```
cyl: 0    16  24    43 50       82          140    170   190  199
SSTF:            43◄─50
            24◄──43
         16◄─24
         16─────────────────────►82─────────►140───►170──►190

SCAN:                  50───────►82─────────►140───►170──►190──►199
                    43◄──────────────────────────────────────────199
               24◄─43
           16◄─24

C-LOOK:                50───────►82─────────►140───►170──►190
           16◄═══════════════════════════════════════════════190  (jump)
           16─►24──►43
```

## C. Functions Used

| Function | Kind | Purpose |
|---|---|---|
| `main()` | user | Read input, sort requests, find split point, call the 3 algorithms |
| `sstf()` | user | Repeatedly pick the nearest unserved request |
| `scan(int start)` | user | Go up to the disk end, then come back down |
| `clook(int start)` | user | Go up to the highest request, jump to the lowest, go up again |
| `abs(x)` | `<stdlib.h>` | Absolute value → distance between two cylinders |
| `printf` / `scanf` | `<stdio.h>` | I/O |

**Global variables** (shared by all functions, so no parameters needed):
`req[30]` (request queue), `n` (count), `head` (initial position), `size` (number of cylinders).

## D. Execution Flow (Call Chain)

```
main()
 ├─ scanf size, head, n, req[0..n-1]
 ├─ bubble sort req[] ascending          e.g. 16 24 43 82 140 170 190
 ├─ start = first index with req[start] ≥ head
 │         (everything left of start is BELOW head, from start onward is ABOVE)
 │
 ├─ sstf() ─────────────────────────────────────────────┐
 │    pos = head                                         │
 │    repeat n times:                                    │
 │      ├ scan all j not done → find min |req[j] − pos|  │
 │      ├ total += min; pos = req[idx]; done[idx] = 1    │
 │      └ printf " -> pos"                               │
 │    printf total                                       │
 │  ◄─── returns ─────────────────────────────────────────┘
 │
 ├─ scan(start) ────────────────────────────────────────┐
 │    pos = head                                         │
 │    UP:   for i = start .. n-1  → visit req[i]         │
 │    END:  visit size-1 (disk boundary)                 │
 │    DOWN: for i = start-1 .. 0  → visit req[i]         │
 │    printf total                                       │
 │  ◄─── returns ─────────────────────────────────────────┘
 │
 ├─ clook(start) ───────────────────────────────────────┐
 │    pos = head                                         │
 │    UP:   for i = start .. n-1  → visit req[i]         │
 │    JUMP+UP: for i = 0 .. start-1 → visit req[i]       │
 │    printf total                                       │
 │  ◄─── returns ─────────────────────────────────────────┘
 │
 └─ return 0
```

**Short chain:** `main → scanf → sort → find start → sstf → printf → main → scan(start) → printf → main → clook(start) → printf → main → return`

**Why sort first?** Once sorted, "requests above the head in increasing order" is just `req[start..n-1]` and "below the head in decreasing order" is `req[start-1..0]`. SCAN and C-LOOK become simple loops. (SSTF doesn't need sorting, but it doesn't hurt — it also makes ties resolve to the lower cylinder.)

## E. Line-by-Line Explanation

| Line | Code | Explanation |
|---|---|---|
| 1 | `#include <stdio.h>` | I/O. |
| 2 | `#include <stdlib.h>` | `abs`. |
| 4 | `int req[30], n, head, size;` | **Globals** used by all functions: up to 30 requests, request count, initial head, disk size. |
| **sstf()** | | |
| 6 | `void sstf() {` | Shortest Seek Time First. |
| 7 | `int done[30] = {0}, pos = head, total = 0, idx = 0;` | `done[j]` = request j served? `pos` = current head; `total` = movement so far; `idx` = chosen request. |
| 8 | `printf("\nSSTF: %d", pos);` | Start the path with the initial head. |
| 9 | `for (int i = 0; i < n; i++) {` | Serve exactly `n` requests, one per iteration. |
| 10 | `int min = 99999;` | "Infinity" — any real distance will be smaller. |
| 11 | `for (int j = 0; j < n; j++) {` | Look at every request… |
| 12 | `if (!done[j] && abs(req[j] - pos) < min) {` | …that isn't served yet and is **closer** than the best so far. |
| 13–14 | `min = abs(req[j] - pos); idx = j;` | Remember the new nearest distance and its index. Strict `<` means on a tie the first (smaller cylinder) wins. |
| 17 | `total += min;` | Add the seek distance. |
| 18 | `pos = req[idx];` | Move the head there. |
| 19 | `done[idx] = 1;` | Mark served. |
| 20 | `printf(" -> %d", pos);` | Print path. |
| 22 | `printf("\nTotal head movement = %d\n", total);` | Final result for SSTF. |
| **scan()** | | |
| 25 | `void scan(int start) {` | SCAN (elevator), moving towards **higher** cylinders first. `start` = index of the first request ≥ head. |
| 26 | `int pos = head, total = 0;` | Fresh start from the original head. |
| 27 | `printf("\nSCAN: %d", pos);` | Path start. |
| 28 | `for (int i = start; i < n; i++) {` | Go **up**: serve all requests above the head in increasing order. |
| 29 | `total += req[i] - pos;` | Moving up, so `req[i] ≥ pos` → no `abs` needed. |
| 30–31 | `pos = req[i]; printf(...)` | Move and print. |
| 33 | `total += (size - 1) - pos;` | SCAN continues to the **last cylinder** (`size-1`) even if no request is there. |
| 34–35 | `pos = size - 1; printf(...)` | Head now at the disk end; print it. |
| 36 | `for (int i = start - 1; i >= 0; i--) {` | **Reverse**: serve requests below the original head in decreasing order. |
| 37 | `total += pos - req[i];` | Moving down, so `pos ≥ req[i]`. |
| 38–39 | `pos = req[i]; printf(...)` | Move and print. |
| 41 | `printf("\nTotal head movement = %d\n", total);` | SCAN result. |
| **clook()** | | |
| 44 | `void clook(int start) {` | Circular LOOK, moving up. |
| 45–46 | `int pos = head, total = 0; printf(...)` | Fresh start. |
| 47–51 | `for (int i = start; i < n; i++) { total += req[i] - pos; pos = req[i]; … }` | Go up to the **highest request** (not the disk end — that's the "LOOK" part). |
| 52 | `for (int i = 0; i < start; i++) {` | **Jump** to the lowest request and continue **upward** again (the "Circular" part). |
| 53 | `total += abs(req[i] - pos);` | `abs` needed: the first step here is the big jump **down** (e.g. 190 → 16); after that the moves are up. This program **counts the jump** in the total. |
| 54–55 | `pos = req[i]; printf(...)` | Move and print. |
| 57 | `printf("\nTotal head movement = %d\n", total);` | C-LOOK result. |
| **main()** | | |
| 60 | `int main() {` | Entry point. |
| 61–68 | `printf/scanf …` | Read disk size, head, number of requests and the requests themselves. |
| 71–77 | bubble sort on `req` | Sort requests **ascending** (same bubble sort as Assignment 2, swap with `t`). |
| 79 | `int start = 0;` | Will become the split index. |
| 80 | `while (start < n && req[start] < head) start++;` | Skip all requests **below** the head. Now `req[start]` is the first request ≥ head (or `start == n` if all are below). |
| 82 | `sstf();` | Run SSTF (uses globals). |
| 83 | `scan(start);` | Run SCAN. |
| 84 | `clook(start);` | Run C-LOOK. Each function starts again from the original `head` because `head` is never modified. |
| 86 | `return 0;` | End. |

## F. Dry Run

**Input:** disk size = 200 (cylinders 0–199), head = 50, n = 7, requests = `82 170 43 140 24 16 190`

**After sorting:** `req = [16, 24, 43, 82, 140, 170, 190]` (indices 0–6)
**Split:** `16<50 → 24<50 → 43<50 → 82≥50` stop → **`start = 3`**

### SSTF

| Step | pos | Distances to unserved requests | Chosen | Move | Total |
|---|---|---|---|---|---|
| 1 | 50 | 16:34 24:26 **43:7** 82:32 140:90 170:120 190:140 | 43 | 7 | 7 |
| 2 | 43 | 16:27 **24:19** 82:39 … | 24 | 19 | 26 |
| 3 | 24 | **16:8** 82:58 … | 16 | 8 | 34 |
| 4 | 16 | **82:66** 140:124 … | 82 | 66 | 100 |
| 5 | 82 | **140:58** 170:88 190:108 | 140 | 58 | 158 |
| 6 | 140 | **170:30** 190:50 | 170 | 30 | 188 |
| 7 | 170 | **190:20** | 190 | 20 | **208** |

### SCAN (towards higher cylinders)

| Move | From → To | Distance | Total |
|---|---|---|---|
| up | 50 → 82 | 32 | 32 |
| up | 82 → 140 | 58 | 90 |
| up | 140 → 170 | 30 | 120 |
| up | 170 → 190 | 20 | 140 |
| to end | 190 → 199 | 9 | 149 |
| down | 199 → 43 | 156 | 305 |
| down | 43 → 24 | 19 | 324 |
| down | 24 → 16 | 8 | **332** |

Shortcut formula: `(size−1 − head) + (size−1 − lowest) = 149 + 183 = 332` ✓

### C-LOOK

| Move | From → To | Distance | Total |
|---|---|---|---|
| up | 50 → 82 → 140 → 170 → 190 | 140 | 140 |
| jump | 190 → 16 | 174 | 314 |
| up | 16 → 24 | 8 | 322 |
| up | 24 → 43 | 19 | **341** |

**Exact output:**
```
SSTF: 50 -> 43 -> 24 -> 16 -> 82 -> 140 -> 170 -> 190
Total head movement = 208

SCAN: 50 -> 82 -> 140 -> 170 -> 190 -> 199 -> 43 -> 24 -> 16
Total head movement = 332

C-LOOK: 50 -> 82 -> 140 -> 170 -> 190 -> 16 -> 24 -> 43
Total head movement = 341
```

| Algorithm | Total movement | Observation |
|---|---|---|
| SSTF | **208** (best here) | Greedy, but 190 waited until the end — starvation risk |
| SCAN | 332 | Wasted 9 cylinders going to 199 with no request there |
| C-LOOK | 341 | Big jump counted; gives most uniform waiting time |

## G. Compile & Run

```bash
gcc 8_disk.c -o disk
```
```bash
./disk
```
Input for the example:
```
200
50
7
82 170 43 140 24 16 190
```

## H. Mistakes, Edge Cases & Viva

- **Some textbooks don't count the C-LOOK jump** in head movement. Then C-LOOK = 341 − 174 = 167. Our program counts it (state which convention you use in the exam).
- **All requests below head** (`start == n`): SCAN goes to the end and comes back; C-LOOK's first loop does nothing and it serves `req[0..n-1]` upward after a jump down.
- **More than 30 requests** overflow `req[30]`.
- **Q: Which algorithm can starve requests?** SSTF.
- **Q: SCAN vs LOOK?** SCAN goes to the physical end of the disk; LOOK reverses at the last request.
- **Q: SCAN vs C-SCAN?** SCAN serves on the way back too; C-SCAN only serves in one direction and jumps back to the start.
- **Q: Why is SCAN called the elevator algorithm?** Like a lift, it keeps going in one direction serving floors, then reverses.
- **Q: Which is best?** Depends on load. SSTF/LOOK are common defaults; C-LOOK gives more uniform wait times under heavy load. Modern SSDs have no seek time, so these matter less there.
- **Q: Why is SSTF not optimal?** It's greedy — a locally closest choice can lead to a longer total path.

---
---

# 9. Quick Revision Sheet

### One-line call chains

| # | Program | Call chain |
|---|---|---|
| 1 | Address book | `while → read ch → case → {>, cat, >>, grep -v + mv, exit}` |
| 2 | Process | `main → fork → child{sort → print → exit} ‖ parent{sort → print → wait → fork(zombie) → sleep → wait → fork(orphan) → exit}` |
| 3 | Producer–consumer | `main → init → create(P) ‖ create(C) → join → destroy`; P: `wait(empty) → lock → put → unlock → post(full)`; C: `wait(full) → lock → take → unlock → post(empty)` |
| 4 | Dining | `main → init(room=4, fork=1) → create×5 → join×5`; phil: `wait(room) → wait(L) → wait(R) → eat → post(R) → post(L) → post(room)` |
| 5 | Banker's | `main → read → need = max − alloc → safety loop (need ≤ avail ⇒ avail += alloc) → SAFE/UNSAFE` |
| 6 | FIFO | P1: `mkfifo → scanf → open(f1,W) → write → open(f2,R) → read → print`; P2: `mkfifo → open(f1,R) → read → count → file → open(f2,W) → write` |
| 7 | Pipes | `main → pipe×2 → fork → parent{close → scanf → write p1 → read p2 → print → wait} ‖ child{close → read p1 → fopen/fread → write p2}` |
| 8 | Disk | `main → read → sort → find start → sstf() → scan(start) → clook(start)` |

### Key formulas & facts

| Topic | Remember |
|---|---|
| `fork()` return | parent: child PID, child: 0, error: −1 |
| Zombie | child dead, parent hasn't `wait`ed |
| Orphan | parent dead, child adopted by init (PID 1) |
| Producer–consumer init | `empty = SIZE`, `full = 0`, `mutex` unlocked; semaphore **before** mutex |
| Dining fix | `room = N−1` → breaks circular wait |
| Coffman | Mutual exclusion, Hold & wait, No preemption, Circular wait |
| Banker's | `Need = Max − Alloc`; if `Need ≤ Work` → `Work += Alloc` |
| Pipe ends | `fd[0]` read, `fd[1]` write |
| FIFO open | blocks until the other side opens |
| SCAN total (up first) | `(size−1 − head) + (size−1 − lowest)` |
| Access time | seek + rotational latency + transfer |

### Compile cheat-sheet

```bash
gcc file.c -o out
```
```bash
gcc file.c -o out -pthread
```
(`-pthread` for Assignments 3 and 4.)
