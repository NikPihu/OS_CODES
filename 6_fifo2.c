#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    char str[500], res[500];
    int fd1, fd2, c = 0, w = 0, l = 0;
    FILE *fp;

    mkfifo("fifo1", 0666);
    mkfifo("fifo2", 0666);

    // Read sentences from fifo1
    fd1 = open("fifo1", O_RDONLY);
    read(fd1, str, sizeof(str));
    close(fd1);

    // Count characters, words, lines
    for (int i = 0; str[i] != '\0'; i++) {
        c++;
        if (str[i] == '\n') l++;
        if (str[i] != ' ' && str[i] != '\n' && str[i] != '\t' &&
            (i == 0 || str[i - 1] == ' ' || str[i - 1] == '\n' || str[i - 1] == '\t'))
            w++;
    }
    if (c > 0 && str[c - 1] != '\n') l++;

    // Write output to file
    fp = fopen("result.txt", "w");
    fprintf(fp, "Characters: %d\nWords: %d\nLines: %d\n", c, w, l);
    fclose(fp);

    // Read output from file
    fp = fopen("result.txt", "r");
    int n = fread(res, 1, sizeof(res) - 1, fp);
    res[n] = '\0';
    fclose(fp);

    // Write file contents to fifo2
    fd2 = open("fifo2", O_WRONLY);
    write(fd2, res, strlen(res) + 1);
    close(fd2);

    printf("Analysis written to result.txt and sent back via fifo2.\n");
    return 0;
}


/*==========================================================
  THEORY - ASSIGNMENT 6
============================================================
  B.1 Inter-Process Communication (IPC)
  -------------------------------------
  Processes have separate memory (Assignment 2), so they need kernel help to exchange data. IPC mechanisms:

  | Mechanism | Between | Direction | Has a name in filesystem? |
  |---|---|---|---|
  | Pipe (pipe()) | Related processes (parent/child) | One-way per pipe | No |
  | FIFO / named pipe (mkfifo()) | Any processes | One-way per FIFO | Yes |
  | Message queue | Any | Both | Key-based |
  | Shared memory | Any | Both (fastest) | Key-based |
  | Socket | Any, even across machines | Both | Address-based |
  | Signal | Any | Notification only | — |

  B.2 FIFO (named pipe)
  ---------------------
  - A special file in the filesystem (ls -l shows it as type p: prw-rw-r-- fifo1).
  - Data written by one process is read by another in First-In-First-Out order.
  - Data is not stored on disk — it goes through a kernel buffer. The file only acts as a *meeting point / name*.
  - Once read, data is removed from the FIFO.

  B.3 Half-duplex vs full-duplex
  ------------------------------
  - A single FIFO is half-duplex (data flows one way at a time).
  - Full-duplex (both ways) needs two FIFOs: fifo1 for P1→P2, fifo2 for P2→P1.

  B.4 Blocking behaviour of open() on a FIFO — very important
  -----------------------------------------------------------
  - open(fifo, O_WRONLY) blocks until some process opens it for reading.
  - open(fifo, O_RDONLY) blocks until some process opens it for writing.
  - read() blocks until data is available (returns 0 when all writers have closed = EOF).

  That's how the two programs synchronize automatically: whichever starts first just waits for the other.

  B.5 Permissions 0666
  --------------------
  Octal: owner rw- (6), group rw- (6), others rw- (6). The real permission is 0666 & ~umask (typically 0664).

  B.6 Low-level I/O vs stdio
  --------------------------
  | Low-level (system calls) | stdio (library, buffered) |
  |---|---|
  | open, read, write, close | fopen, fread, fprintf, fclose |
  | works with int fd | works with FILE * |
  | used here for the FIFOs | used here for result.txt |

  B.7 Counting words, characters, lines
  -------------------------------------
  - Characters: every byte counts (including spaces and \n).
  - Lines: count \n, plus 1 if the text doesn't end with \n (the last unfinished line).
  - Words: count the positions where a word starts = a non-whitespace character that is either the first character or comes right after whitespace.
==========================================================*/
