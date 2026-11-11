#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int p1[2], p2[2], n;
    char fname[100], buf[1000];
    FILE *fp;

    pipe(p1); // p1: parent -> child (file name)
    pipe(p2); // p2: child -> parent (file content)

    if (fork() > 0) {
        // Parent Process
        close(p1[0]);
        close(p2[1]);

        printf("Enter file name: ");
        scanf("%s", fname);
        write(p1[1], fname, strlen(fname) + 1);

        read(p2[0], buf, sizeof(buf));
        printf("\nFile contents received by parent:\n%s\n", buf);
        wait(NULL);
    } else {
        // Child Process
        close(p1[1]);
        close(p2[0]);

        read(p1[0], fname, sizeof(fname));

        fp = fopen(fname, "r");
        if (fp == NULL) {
            strcpy(buf, "File not found!");
        } else {
            n = fread(buf, 1, sizeof(buf) - 1, fp);
            buf[n] = '\0';
            fclose(fp);
        }
        write(p2[1], buf, strlen(buf) + 1);
    }
    return 0;
}


//compile
// # Create a test file first:
// echo "Operating Systems Lab: Full duplex IPC using ordinary pipes." > testfile.txt

// # Compile and run:
// gcc 7_pipes.c -o pipes
// ./pipes

//input
// testfile.txt


/*==========================================================
  THEORY - ASSIGNMENT 7
============================================================
  B.1 What is a pipe?
  -------------------
  - A unidirectional byte channel in the kernel with two ends:
    - fd[0] → read end
    - fd[1] → write end
    (Memory trick: 0 = input like stdin, 1 = output like stdout.)
  - Data comes out in the same order it went in (FIFO). Kernel buffer is typically 64 KB on Linux.
  - A pipe has no name, so only processes that inherit the fds (via fork) can use it → related processes only.

  B.2 How fork makes a pipe usable
  --------------------------------
  After pipe(p1):              After fork():
   Parent                       Parent              Child
    p1[0] ─┐                     p1[0] ─┐  ┌─ p1[0]
    p1[1] ─┼─► [kernel buffer]   p1[1] ─┼──┼─ p1[1]   ──► same kernel pipe
                                         └──┘
  Both processes now hold both ends. Each must close the ends it doesn't use.

  B.3 Why close unused ends?
  --------------------------
  - EOF detection: read() returns 0 (EOF) only when all write ends are closed. If the reader still holds its own copy of the write end, it will never see EOF and may block forever.
  - Clarity & resources: frees file descriptors and makes the direction explicit.
  - SIGPIPE: writing to a pipe with no open read end kills the writer with SIGPIPE.

  B.4 Full duplex needs two pipes
  -------------------------------
  A single pipe is one-way. If both processes read and wrote the same pipe, one might read back its own data. So:
  - p1: parent → child (parent writes p1[1], child reads p1[0])
  - p2: child → parent (child writes p2[1], parent reads p2[0])

  B.5 Blocking rules
  ------------------
  | Situation | Result |
  |---|---|
  | read on empty pipe, writer still open | blocks |
  | read on empty pipe, all writers closed | returns 0 (EOF) |
  | write to full pipe | blocks |
  | write with all readers closed | SIGPIPE / error EPIPE |
  | Writes ≤ PIPE_BUF (4096 bytes) | atomic (not mixed with other writes) |
==========================================================*/


/*==========================================================
  FUNCTIONS & SYSTEM CALLS - ASSIGNMENT 7
============================================================
  | Function | Header | Prototype | Returns | Used for |
  |---|---|---|---|---|
  | pipe | <unistd.h> | int pipe(int fd[2]); | 0 / −1 | Create a pipe; fills fd[0] (read), fd[1] (write) |
  | fork | <unistd.h> | pid_t fork(void); | child PID / 0 / −1 | Create the child that inherits the pipes |
  | close | <unistd.h> | int close(int fd); | 0 | Close unused ends |
  | write | <unistd.h> | ssize_t write(int fd, const void *buf, size_t n); | bytes | Send through pipe |
  | read | <unistd.h> | ssize_t read(int fd, void *buf, size_t n); | bytes / 0 at EOF | Receive (blocks until data) |
  | wait | <sys/wait.h> | pid_t wait(int *st); | child PID | Parent reaps child |
  | fopen / fread / fclose | <stdio.h> | — | — | Child reads the requested file |
  | strlen / strcpy | <string.h> | — | — | Lengths, error message |
==========================================================*/


/*==========================================================
  EXECUTION FLOW (CALL CHAIN) - ASSIGNMENT 7
============================================================
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

  Short chain: main → pipe(p1) → pipe(p2) → fork → {parent: close×2 → scanf → write(p1) → read(p2) → printf → wait} ‖ {child: close×2 → read(p1) → fopen → fread → fclose → write(p2) → exit}
==========================================================*/


/*==========================================================
  DRY RUN - ASSIGNMENT 7
============================================================
  Suppose notes.txt contains:
  Operating Systems
  Lab 7

  | Step | Parent | Pipe p1 | Pipe p2 | Child |
  |---|---|---|---|---|
  | 1 | pipe, pipe, fork | empty | empty | created |
  | 2 | closes p1[0], p2[1] | | | closes p1[1], p2[0] |
  | 3 | prints prompt, waits for keyboard | | | blocked in read(p1[0]) |
  | 4 | user types notes.txt; write(p1[1]) | notes.txt\0 | | |
  | 5 | blocked in read(p2[0]) | → consumed | | reads notes.txt |
  | 6 | | | | fopen OK, fread 24 bytes |
  | 7 | | | Operating Systems\nLab 7\n\0 | write(p2[1]), returns, exits |
  | 8 | read returns, prints contents | | → consumed | (zombie briefly) |
  | 9 | wait(NULL) reaps child, exits | | | gone |

  Output:
  Enter file name: notes.txt

  File contents received by parent:
  Operating Systems
  Lab 7

  With a wrong name (abc.txt):
  Enter file name: abc.txt

  File contents received by parent:
  File not found!
==========================================================*/
