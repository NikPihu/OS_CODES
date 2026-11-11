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
