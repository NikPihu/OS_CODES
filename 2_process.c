#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void sort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
}

void print(int arr[], int n) {
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");
}

int main() {
    int n;
    printf("Enter number of elements: ");
    scanf("%d", &n);
    int arr[n];
    printf("Enter %d integers: ", n);
    for (int i = 0; i < n; i++) scanf("%d", &arr[i]);

    // 1. Fork and Wait
    pid_t pid = fork();

    if (pid == 0) {
        // Child Process
        printf("\n[Child] PID: %d, PPID: %d\n", getpid(), getppid());
        sort(arr, n);
        printf("[Child] Sorted array: ");
        print(arr, n);
        exit(0);
    } else {
        // Parent Process
        printf("\n[Parent] PID: %d, Child PID: %d\n", getpid(), pid);
        sort(arr, n);
        printf("[Parent] Sorted array: ");
        print(arr, n);
        wait(NULL); // wait for sorting child
        printf("[Parent] Child finished.\n");

        // 2. Zombie State Demo
        printf("\n--- Demonstrating Zombie State ---\n");
        pid_t z = fork();
        if (z == 0) {
            printf("[Zombie Child] PID %d exiting now...\n", getpid());
            exit(0);
        } else {
            printf("[Parent] Sleeping 2s without wait(). Child %d is ZOMBIE.\n", z);
            sleep(2);
            wait(NULL);
            printf("[Parent] Reaped zombie child.\n");
        }

        // 3. Orphan State Demo
        printf("\n--- Demonstrating Orphan State ---\n");
        pid_t o = fork();
        if (o == 0) {
            sleep(2); // wait for parent to exit
            printf("[Orphan Child] PID: %d, Adopted by Parent PID: %d\n", getpid(), getppid());
            exit(0);
        } else {
            printf("[Parent] Exiting now. Child %d becomes ORPHAN.\n", o);
            exit(0);
        }
    }
    return 0;
}


//compile
// gcc 2_process.c -o 2_process
// ./2_process


// input 
// 5 
// 64 25 12 22 11


/*==========================================================
  THEORY - ASSIGNMENT 2
============================================================
  B.1 Process Control Block (PCB)
  -------------------------------
  The kernel keeps a PCB (in Linux: task_struct) for every process, containing:
  PID, parent PID (PPID), state, CPU registers & program counter, memory info, open-file table, scheduling info, exit status.

  B.2 fork() — creating a process
  -------------------------------
  Before fork:              After fork:
  ┌────────────────┐        ┌────────────────┐      ┌────────────────┐
  │ Parent PID=100 │        │ Parent PID=100 │      │ Child  PID=101 │
  │ arr = [3,1,2]  │  ──►   │ arr = [3,1,2]  │      │ arr = [3,1,2]  │
  │ pc → fork()    │        │ fork() → 101   │      │ fork() → 0     │
  └────────────────┘        └────────────────┘      └────────────────┘
  - Called once, returns twice (once in each process).
  - Return value: > 0 in parent (= child's PID), 0 in child, −1 on failure (no child created).
  - The child is an almost exact copy: same code, same variable values, same open files, *same program counter* (both continue from the line after fork).
  - Memory is not shared afterwards. A change in one process is invisible to the other (Copy-On-Write).
  - Execution order after fork is decided by the scheduler → output can interleave differently every run.

  B.3 wait() — waiting for a child
  --------------------------------
  - wait(NULL) blocks the caller until any one child terminates, then removes the child's PCB from the process table (reaping).
  - Passing NULL means "I don't care about the exit status". Passing &status would let you read it with WEXITSTATUS(status).
  - Returns the PID of the child that ended, or −1 if there are no children.

  B.4 exit() — terminating a process
  ----------------------------------
  - exit(0) flushes stdio buffers, closes files, and terminates the process. 0 = success.
  - The process becomes a zombie until its parent calls wait().

  B.5 Zombie process
  ------------------
  - Definition: a process that has finished execution but still has an entry in the process table because the parent hasn't read its exit status yet.
  - State shown by ps as Z / <defunct>.
  - Holds no memory or CPU — only a PID and exit status. But too many zombies can exhaust the PID table.
  - Fix: parent calls wait()/waitpid(), or parent dies (then init adopts and reaps it).

  B.6 Orphan process
  ------------------
  - Definition: a process whose parent has terminated while it is still running.
  - The kernel re-parents it to init (PID 1) — or on modern systems to systemd/a "subreaper" process. So getppid() changes from the old parent PID to 1 (or the subreaper's PID).
  - Orphans are not harmful: init will wait() for them when they finish.

  | | Zombie | Orphan |
  |---|---|---|
  | Who is dead? | the child | the parent |
  | Who is alive? | the parent | the child |
  | Running code? | No | Yes |
  | Harmful? | Yes (wastes PID slot) | No |
  | Cleaned by | parent's wait() | init adopts and reaps |

  B.7 Bubble sort (used in sort)
  ------------------------------
  Repeatedly compare neighbours and swap if out of order. After pass *i*, the largest *i+1* elements are in their final place at the end. Time O(n²), space O(1).
==========================================================*/


/*==========================================================
  FUNCTIONS & SYSTEM CALLS - ASSIGNMENT 2
============================================================
  | Function | Header | Prototype | Returns | Used for |
  |---|---|---|---|---|
  | fork | <unistd.h> | pid_t fork(void); | child PID / 0 / −1 | Create child |
  | getpid | <unistd.h> | pid_t getpid(void); | own PID | Print identity |
  | getppid | <unistd.h> | pid_t getppid(void); | parent's PID | Show parent / adoption |
  | sleep | <unistd.h> | unsigned sleep(unsigned sec); | 0 when done | Create timing for zombie/orphan |
  | wait | <sys/wait.h> | pid_t wait(int *status); | dead child's PID | Block + reap child |
  | exit | <stdlib.h> | void exit(int status); | never returns | End process |
  | printf/scanf | <stdio.h> | — | — | I/O |
  | sort | user-defined | void sort(int arr[], int n) | — | Bubble sort |
  | print | user-defined | void print(int arr[], int n) | — | Print array |

  pid_t is just a signed integer type used for process IDs.
==========================================================*/


/*==========================================================
  EXECUTION FLOW (CALL CHAIN) - ASSIGNMENT 2
============================================================
  Function call chain:
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

  Short chain to memorise:
  main → fork → {child: sort → print → exit} ‖ {parent: sort → print → wait → fork(zombie) → sleep → wait → fork(orphan) → exit}

  Process tree over time:
  t=0      P(100)
  t=1      P(100) ── C1(101)                 sort children
  t=2      P(100)                            C1 reaped by wait
  t=3      P(100) ── Z(102) <defunct>        zombie for 2 seconds
  t=5      P(100)                            Z reaped by wait
  t=6      P(100) ── O(103)
  t=6.1    init(1) ── O(103)                 P exited → O adopted
  t=8      O prints PPID=1, exits; init reaps it
==========================================================*/


/*==========================================================
  DRY RUN - ASSIGNMENT 2
============================================================
  Input: n = 5, array 4 1 3 5 2. Assume parent PID = 1000.

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

  Bubble-sort trace on [4,1,3,5,2]:

  | Pass i | Array after pass | Comment |
  |---|---|---|
  | 0 | 1 3 4 2 5 | 5 bubbled to the end |
  | 1 | 1 3 2 4 5 | 4 in place |
  | 2 | 1 2 3 4 5 | 3 in place |
  | 3 | 1 2 3 4 5 | done |

  Note: Order of [Parent] and [Child] lines can change between runs — it depends on the scheduler.
==========================================================*/
