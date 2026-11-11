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
