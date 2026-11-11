# OS Laboratory: Master Theory, Code Walkthrough & Dry Run Guide

> **Prepared for**: Quick memorization, deep conceptual understanding, and practical viva exams.  
> **Source Codes**: Found in [`short_corrected_code/`](./) — clean, minimal lines, zero bloat, 100% syllabus aligned.

---

# Table of Contents
1. [Assignment 1: Shell Programming — Address Book (CO1)](#assignment-1-shell-programming--address-book-co1)
2. [Assignment 2: Process Management — Fork, Wait, Zombie & Orphan (CO2)](#assignment-2-process-management--fork-wait-zombie--orphan-co2)
3. [Assignment 3: Thread Synchronization — Producer-Consumer (CO3)](#assignment-3-thread-synchronization--producer-consumer-co3)
4. [Assignment 4: Deadlock Avoidance — Dining Philosophers (CO3, CO4)](#assignment-4-deadlock-avoidance--dining-philosophers-co3-co4)
5. [Assignment 5: Deadlock Avoidance — Banker's Algorithm (CO4)](#assignment-5-deadlock-avoidance--bankers-algorithm-co4)
6. [Assignment 6: Interprocess Communication — Full Duplex FIFOs (CO5)](#assignment-6-interprocess-communication--full-duplex-fifos-co5)
7. [Assignment 7: Interprocess Communication — Full Duplex Pipes (CO5)](#assignment-7-interprocess-communication--full-duplex-pipes-co5)
8. [Assignment 8: Disk Scheduling — SSTF, SCAN, C-LOOK (CO5)](#assignment-8-disk-scheduling--sstf-scan-c-look-co5)

---

# Assignment 1: Shell Programming — Address Book (CO1)

### 1. Theory & Core Concepts
- **What is a Shell Script?** A script executed by the Unix shell interpreter (`/bin/bash`). It automates terminal commands.
- **Redirection Operators**:
  - `> file` : Overwrites file or truncates to 0 bytes (used in Create).
  - `>> file`: Appends output to the end of the file (used in Insert).
  - `grep -v "^pattern"` : Filters OUT matching lines. Used with a temporary file (`temp.txt`) to delete or modify lines.
  - `grep -q`: Quiet search. Returns status 0 (true) if found, non-zero if not found without printing text to screen.
- **File Test Operators**:
  - `-s "$file"`: Checks if file exists and has a size greater than zero (not empty).

---

### 2. Code Walkthrough (`1_address_book.sh`)

#### Level 1: Architecture Flow
```
Start -> Infinite Loop (while true) -> Show Menu -> Read Choice ($ch)
       -> Case a/1: > address.txt
       -> Case b/2: Check -s, cat address.txt
       -> Case c/3: Read Name Phone City -> Append (>>) to address.txt
       -> Case d/4: Check grep -q -> grep -v to temp.txt -> mv to address.txt
       -> Case e/5: Check grep -q -> grep -v to temp.txt -> mv -> Read new details -> Append
       -> Case f/6: exit 0
```

#### Level 2: Line-by-Line Breakdown
- **Line 6: `file="address.txt"`**: Stores database filename in variable.
- **Line 8: `while true; do ... done`**: Keeps menu running until user chooses Exit.
- **Line 14: `a|1) > $file ;;`**: The `>` alone empties/creates the file.
- **Line 17: `if [ -s "$file" ]; then cat $file`**: Ensures we only `cat` when there is actual data.
- **Line 26: `echo "$name $phone $city" >> $file`**: Saves space-separated record.
- **Line 29: `if grep -q "^$name " $file; then`**: 
  - `^$name `: The `^` anchors the search to the start of the line, followed by a space. This prevents substring false positives (e.g., searching for "Al" won't match "Alex").
- **Line 30: `grep -v "^$name " $file > temp.txt && mv temp.txt $file`**:
  - Drops the matched record into `temp.txt`, then renames it back to `address.txt`.

---

### 3. Step-by-Step Dry Run
Suppose we insert two records:
1. `c` $\rightarrow$ Name: `John`, Phone: `111`, City: `Pune` $\rightarrow$ `address.txt` contains: `John 111 Pune`
2. `c` $\rightarrow$ Name: `Mary`, Phone: `222`, City: `Mumbai` $\rightarrow$ `address.txt` contains:
   ```
   John 111 Pune
   Mary 222 Mumbai
   ```
3. `d` $\rightarrow$ Delete `John`:
   - `grep -q "^John "` succeeds.
   - `grep -v "^John "` writes only `Mary 222 Mumbai` to `temp.txt`.
   - `mv temp.txt address.txt` replaces file. `John` is deleted!

---

### 4. Memory Blueprint (How to write from memory)
1. Initialize `file="address.txt"`.
2. Wrap `case $ch in` inside `while true`.
3. Remember the 6 options: `a|1`, `b|2`, `c|3`, `d|4`, `e|5`, `f|6`.
4. Delete and Modify use the same trick: `grep -v "^$name " $file > temp.txt && mv temp.txt $file`.

---

### 5. Viva Questions
- **Q: Why do we write to `temp.txt` before renaming?**  
  *A: You cannot read and write to the same file in a single pipeline (`grep -v ... file > file` truncates the file to 0 bytes before grep can read it).*
- **Q: What does `^` mean in grep?**  
  *A: It is a regular expression anchor asserting the beginning of a line.*

---
---

# Assignment 2: Process Management — Fork, Wait, Zombie & Orphan (CO2)

### 1. Theory & Core Concepts
- **`fork()`**: System call that creates an exact duplicate child process.
  - Return value in Parent: Child's PID ($> 0$).
  - Return value in Child: $0$.
  - Return value on failure: $-1$.
- **`wait(NULL)`**: Suspends parent execution until one of its children terminates. It reaps the child's exit status from the kernel process table.
- **Zombie State (`<defunct>`)**:
  - Occurs when a child process finishes execution (`exit(0)`), but the parent hasn't yet called `wait()`.
  - The child's code and data are freed, but its PID and exit status remain in the process table so parent can read it.
- **Orphan State**:
  - Occurs when a parent process terminates before its child process.
  - The operating system reparents the orphan child to `init` (PID 1) or `systemd`.

---

### 2. Code Walkthrough (`2_process.c`)

#### Level 1: Architecture Flow
```
main() -> Read array
       -> fork()
          ├── pid == 0 (Child)  -> sort(arr) -> print() -> exit(0)
          └── pid > 0  (Parent) -> sort(arr) -> print() -> wait(NULL)
                                -> Zombie Demo: fork() -> child exits immediately -> parent sleeps 2s -> wait()
                                -> Orphan Demo: fork() -> child sleeps 2s -> parent exits immediately
```

#### Level 2: Line-by-Line Breakdown
- **Lines 6-14: `void sort(int arr[], int n)`**: Standard Bubble Sort algorithm.
- **Line 33: `pid_t pid = fork();`**: Duplicates parent process into child.
- **Line 35: `if (pid == 0)`**: Child's branch. Executes sorting, prints sorted array, then `exit(0)`.
- **Line 43: `wait(NULL);`**: Parent pauses here until Child finishes sorting.
- **Lines 49-57 (Zombie Demo)**:
  - `pid_t z = fork();`
  - Child calls `exit(0);` immediately $\rightarrow$ Child is dead.
  - Parent calls `sleep(2);` WITHOUT calling wait $\rightarrow$ Child entry remains in OS process table as ZOMBIE.
  - Parent calls `wait(NULL);` $\rightarrow$ Zombie is cleaned up.
- **Lines 60-68 (Orphan Demo)**:
  - `pid_t o = fork();`
  - Child calls `sleep(2);` $\rightarrow$ Child waits.
  - Parent calls `exit(0);` immediately $\rightarrow$ Parent dies.
  - Child wakes up: its original parent is dead, so `getppid()` now returns `1` (or systemd PID).

---

### 3. Step-by-Step Dry Run
- Input: `arr = [30, 10, 20]`, `n = 3`.
- `fork()` runs. Two processes exist: Parent (PID 5000), Child (PID 5001).
- Child (5001): Sorts `[10, 20, 30]`, prints, calls `exit(0)`.
- Parent (5000): Sorts `[10, 20, 30]`, prints, waits for 5001.
- Zombie: Parent forks 5002. 5002 exits. Parent sleeps 2s. Process table shows `PID 5002 [2_process] <defunct>`. Parent calls `wait(NULL)`, entry removed.
- Orphan: Parent forks 5003. Parent exits. 5003 wakes up, checks `getppid()` $\implies$ returns 1.

---

### 4. Memory Blueprint
1. Write helper `sort()` and `print()`.
2. Three forks in sequence:
   - Fork 1: Child sorts & exits; Parent sorts & waits.
   - Fork 2 (Zombie): Child exits; Parent sleeps then waits.
   - Fork 3 (Orphan): Child sleeps then prints `getppid()`; Parent exits.

---

### 5. Viva Questions
- **Q: Why are zombie processes bad if left unchecked?**  
  *A: They occupy PID entries in the kernel's process table. If too many zombies accumulate, the OS runs out of available PIDs.*
- **Q: Who adopts an orphan process?**  
  *A: The `init` process (PID 1) or `systemd` adopts it and automatically reaps it when it finishes.*

---
---

# Assignment 3: Thread Synchronization — Producer-Consumer (CO3)

### 1. Theory & Core Concepts
- **Race Condition**: A situation where multiple threads access and modify shared data concurrently, and the final outcome depends on thread scheduling order.
- **Critical Section**: The part of code where shared resources (e.g., `buffer[]`) are accessed.
- **Mutex (`pthread_mutex_t`)**: A mutual exclusion lock. Only ONE thread can hold the lock at any time.
- **Counting Semaphores**:
  - `empty`: Initialized to `SIZE` (5). Tracks how many empty slots are available for producers.
  - `full`: Initialized to `0`. Tracks how many filled slots are available for consumers.
- **Order of Operations**:
  - Producer: `sem_wait(&empty)` $\rightarrow$ `pthread_mutex_lock(&mutex)` $\rightarrow$ write $\rightarrow$ `pthread_mutex_unlock(&mutex)` $\rightarrow$ `sem_post(&full)`.
  - Always wait on semaphore BEFORE locking mutex to avoid deadlock!

---

### 2. Code Walkthrough (`3_producer_consumer.c`)

#### Level 1: Architecture Flow
```
Shared: buffer[5], in=0, out=0, mutex, empty=5, full=0
Producer Thread                     Consumer Thread
Loop 1..5:                          Loop 1..5:
  sem_wait(&empty)                    sem_wait(&full)
  mutex_lock(&mutex)                  mutex_lock(&mutex)
    buffer[in] = item                   item = buffer[out]
    in = (in + 1) % 5                   out = (out + 1) % 5
  mutex_unlock(&mutex)                mutex_unlock(&mutex)
  sem_post(&full)                     sem_post(&empty)
```

#### Level 2: Line-by-Line Breakdown
- **Line 6: `#define SIZE 5`**: Buffer holds up to 5 items.
- **Line 9: `int in = 0, out = 0;`**: Pointers for circular buffer.
- **Line 14: `sem_wait(&empty);`**: Decrements `empty`. If `empty == 0` (buffer full), producer blocks!
- **Line 15: `pthread_mutex_lock(&mutex);`**: Guarantees no consumer/producer touches buffer at this exact millisecond.
- **Line 18: `in = (in + 1) % SIZE;`**: Advances circular buffer index (wraps from 4 to 0).
- **Line 21: `sem_post(&full);`**: Increments `full`. Wakes up waiting consumer.
- **Line 28: `sem_wait(&full);`**: Decrements `full`. If `full == 0` (buffer empty), consumer blocks!
- **Line 35: `sem_post(&empty);`**: Increments `empty`. Wakes up waiting producer.

---

### 3. Step-by-Step Dry Run
| Action | `in` | `out` | `empty` | `full` | Buffer State |
|---|---|---|---|---|---|
| Initial | 0 | 0 | 5 | 0 | `[0, 0, 0, 0, 0]` |
| Producer produces 10 | 1 | 0 | 4 | 1 | `[10, 0, 0, 0, 0]` |
| Producer produces 20 | 2 | 0 | 3 | 2 | `[10, 20, 0, 0, 0]` |
| Consumer consumes 10 | 2 | 1 | 4 | 1 | `[10, 20, 0, 0, 0]` (item 10 consumed) |

---

### 4. Memory Blueprint
1. Global vars: `buffer[5], in=0, out=0, mutex, sem_t empty, full`.
2. Producer waits `empty`, locks mutex, inserts, unlocks, posts `full`.
3. Consumer waits `full`, locks mutex, removes, unlocks, posts `empty`.
4. In `main`: `sem_init(&empty, 0, 5)`, `sem_init(&full, 0, 0)`, `pthread_mutex_init(&mutex, NULL)`, create, join, destroy.

---

### 5. Viva Questions
- **Q: What happens if `pthread_mutex_lock` is placed before `sem_wait(&empty)`?**  
  *A: Deadlock! If the buffer is full, the producer holds the mutex and goes to sleep waiting for `empty`. The consumer can never access the buffer because the mutex is locked.*
- **Q: Is a mutex a binary semaphore?**  
  *A: They are similar, but a mutex has ownership (only the thread that locked it can unlock it). Any thread can post to a semaphore.*

---
---

# Assignment 4: Deadlock Avoidance — Dining Philosophers (CO3, CO4)

### 1. Theory & Core Concepts
- **The Problem**: 5 philosophers sit around a table with 5 forks. To eat, a philosopher needs both their left and right forks.
- **Deadlock Scenario**: If all 5 philosophers become hungry simultaneously and pick up their left fork, all right forks are unavailable $\implies$ Circular Wait $\implies$ Deadlock!
- **Coffman's 4 Deadlock Conditions**:
  1. Mutual Exclusion
  2. Hold and Wait
  3. No Preemption
  4. Circular Wait
- **The Deadlock-Free Solution (Dijkstra's Room Semaphore)**:
  - We introduce a counting semaphore `room` initialized to $4$ ($N - 1$).
  - At most 4 philosophers can sit at the table simultaneously.
  - **Pigeonhole Principle**: 4 philosophers competing for 5 forks guarantees that at least one philosopher will obtain both forks and eat. Once they finish, they release both forks, allowing others to eat. Circular wait is impossible!

---

### 2. Code Walkthrough (`4_dining.c`)

#### Level 1: Architecture Flow
```
Philosopher(i):
  Thinking
  sem_wait(&room)                 <-- Enter room (max 4 allowed)
  sem_wait(&fork_s[i])            <-- Pick left fork
  sem_wait(&fork_s[(i + 1) % 5])  <-- Pick right fork
  Eating (sleep 1)
  sem_post(&fork_s[(i + 1) % 5])  <-- Release right fork
  sem_post(&fork_s[i])            <-- Release left fork
  sem_post(&room)                 <-- Leave room
```

#### Level 2: Line-by-Line Breakdown
- **Line 5: `sem_t fork_s[5];`**: One binary semaphore for each fork.
- **Line 6: `sem_t room;`**: Counting semaphore initialized to 4.
- **Line 12: `sem_wait(&room);`**: Decrements room count. If 4 philosophers are already seated, the 5th must wait outside.
- **Line 14: `sem_wait(&fork_s[i]);`**: Acquires left fork.
- **Line 15: `sem_wait(&fork_s[(i + 1) % 5]);`**: Acquires right fork using modulo arithmetic.
- **Line 21: `sem_post(&fork_s[(i + 1) % 5]); sem_post(&fork_s[i]);`**: Puts down forks.
- **Line 24: `sem_post(&room);`**: Exits room, allowing a waiting philosopher to enter.

---

### 3. Step-by-Step Dry Run
1. All 5 philosophers get hungry.
2. Philosophers 0, 1, 2, 3 call `sem_wait(&room)`. Room count drops $4 \to 0$.
3. Philosopher 4 calls `sem_wait(&room)` $\rightarrow$ Blocked! Cannot enter.
4. Now only 4 philosophers (0, 1, 2, 3) compete for 5 forks (0, 1, 2, 3, 4).
5. Forks:
   - Phil 0 takes Fork 0, waits for Fork 1.
   - Phil 1 takes Fork 1, waits for Fork 2.
   - Phil 2 takes Fork 2, waits for Fork 3.
   - Phil 3 takes Fork 3 AND Fork 4 (Fork 4 is free because Phil 4 is outside!).
6. Phil 3 eats, finishes, releases Forks 3 & 4, and leaves room.
7. Deadlock is completely prevented.

---

### 4. Memory Blueprint
1. Two semaphores: `fork_s[5]` (init 1), `room` (init 4).
2. Function flow: Think $\rightarrow$ `wait(room)` $\rightarrow$ `wait(fork[i])` $\rightarrow$ `wait(fork[(i+1)%5])` $\rightarrow$ Eat $\rightarrow$ post right $\rightarrow$ post left $\rightarrow$ `post(room)`.
3. In main: `sem_init(&room, 0, 4)`, loop 5 for fork semaphores, create 5 threads, join 5 threads, destroy.

---

### 5. Viva Questions
- **Q: Which Coffman condition does the room semaphore break?**  
  *A: It breaks the Circular Wait condition.*
- **Q: Can starvation occur with this solution?**  
  *A: Deadlock is prevented, but without FIFO queuing on semaphores, a thread might theoretically suffer starvation under extreme scheduling.*

---
---

# Assignment 5: Deadlock Avoidance — Banker's Algorithm (CO4)

### 1. Theory & Core Concepts
- **Deadlock Avoidance**: The OS dynamically examines resource allocations to ensure that a circular wait condition can NEVER develop.
- **Safe State**: A state where there exists at least one order (a **Safe Sequence**) in which all processes can finish execution without deadlocking.
- **Key Vectors & Matrices**:
  - $Allocation[P][R]$: Currently allocated resources.
  - $Max[P][R]$: Maximum demand of each process.
  - $Available[R]$: Currently free resources in the system.
  - $Need[P][R] = Max[P][R] - Allocation[P][R]$.
- **Safety Algorithm Logic**:
  - Find an unfinished process $i$ whose remaining need can be satisfied by currently available resources: $Need[i] \le Available$.
  - If found: assume it runs to completion and releases its resources $\implies Available = Available + Allocation[i]$. Mark process finished.
  - Repeat. If all processes finish, state is **SAFE**!

---

### 2. Code Walkthrough (`5_bankers.c`)

#### Level 1: Architecture Flow
```
Input: P, R, Alloc, Max, Avail
Calculate & Display: Need[i][j] = Max[i][j] - Alloc[i][j]
Safety Loop (run P times):
  For each process i:
    If not done[i] and Need[i] <= Avail:
      Avail = Avail + Alloc[i]
      done[i] = 1
      seq[count++] = i
If count == P -> Print Safe Sequence
Else          -> Print Unsafe State
```

#### Level 2: Line-by-Line Breakdown
- **Line 21: `need[i][j] = max[i][j] - alloc[i][j];`**: Computes required remaining resources.
- **Line 29: `for (int k = 0; k < p; k++)`**: Outer loop ensures we re-check all processes each time new resources are freed.
- **Line 33: `if (need[i][j] > avail[j]) { possible = 0; break; }`**: If any resource requirement exceeds available, process cannot run right now.
- **Line 39: `avail[j] += alloc[i][j]; seq[count++] = i; done[i] = 1;`**: Process finishes, releases all allocated resources back to pool, and joins safe sequence.
- **Line 48: `if (count == p)`**: If all processes could finish, system is safe.

---

### 3. Step-by-Step Dry Run
Given 3 processes ($P_0, P_1, P_2$) and 1 resource type ($R_1$):
- $Alloc = [1, 2, 0]$
- $Max = [3, 2, 2]$
- $Avail = [1]$
- Calculate $Need = Max - Alloc$:
  - $Need[P_0] = 3 - 1 = 2$
  - $Need[P_1] = 2 - 2 = 0$
  - $Need[P_2] = 2 - 0 = 2$

**Iteration 1**:
- Check $P_0$: $Need(2) \le Avail(1)$? No.
- Check $P_1$: $Need(0) \le Avail(1)$? Yes!
  - $P_1$ runs, releases 2. New $Avail = 1 + 2 = 3$.
  - Safe Sequence: `< P1 >`.

**Iteration 2**:
- Check $P_0$: $Need(2) \le Avail(3)$? Yes!
  - $P_0$ runs, releases 1. New $Avail = 3 + 1 = 4$.
  - Safe Sequence: `< P1, P0 >`.

**Iteration 3**:
- Check $P_2$: $Need(2) \le Avail(4)$? Yes!
  - Safe Sequence: `< P1, P0, P2 >`.
- Count = 3 $\implies$ System is in SAFE state!

---

### 4. Memory Blueprint
1. Matrix formula: `need[i][j] = max[i][j] - alloc[i][j]`.
2. Triple loop: `k` from $0..P$, `i` from $0..P$, `j` from $0..R$.
3. Check `need[i][j] <= avail[j]`. If true, `avail += alloc`, `done[i] = 1`, `seq[count++] = i`.
4. If `count == p`, print safe sequence.

---

### 5. Viva Questions
- **Q: Is an unsafe state definitely a deadlock?**  
  *A: No! An unsafe state is not necessarily a deadlock, but it has the potential to lead to one if all processes request their maximum claims.*
- **Q: Why is it called the Banker's Algorithm?**  
  *A: It mimics a bank that never allocates cash exceeding its reserves, ensuring it can always satisfy at least one customer's line of credit.*

---
---

# Assignment 6: Interprocess Communication — Full Duplex FIFOs (CO5)

### 1. Theory & Core Concepts
- **What is a FIFO?** A named pipe created in the filesystem using `mkfifo()`. It allows communication between two completely independent, unrelated processes.
- **Why Two FIFOs for Full Duplex?**
  - A standard pipe/FIFO is unidirectional (half-duplex).
  - To achieve full duplex (two-way communication), we need TWO FIFOs:
    - `fifo1`: Process 1 $\to$ Process 2 (sends sentences).
    - `fifo2`: Process 2 $\to$ Process 1 (sends analysis result).
- **Blocking Nature of FIFOs**:
  - When Process 2 opens `fifo1` for reading (`O_RDONLY`), it automatically blocks/sleeps until Process 1 opens `fifo1` for writing (`O_WRONLY`).

---

### 2. Code Walkthrough (`6_fifo1.c` & `6_fifo2.c`)

#### Level 1: Architecture Flow
```
Process 1 (fifo1.c)                             Process 2 (fifo2.c)
mkfifo("fifo1"), mkfifo("fifo2")               mkfifo("fifo1"), mkfifo("fifo2")
Read sentences from user
Open fifo1 (WRONLY)                            Open fifo1 (RDONLY) [Waits here]
Write sentences ------------(fifo1)----------> Read sentences
Close fifo1                                    Count chars, words, lines
                                               Write to result.txt -> Read from result.txt
Open fifo2 (RDONLY) [Waits here]               Open fifo2 (WRONLY)
Read result <---------------(fifo2)----------- Write file contents
Print result                                   Close fifo2
```

#### Level 2: Line-by-Line Breakdown

**In `6_fifo1.c`:**
- **Line 11: `mkfifo("fifo1", 0666);`**: Creates FIFO special file with read/write permissions.
- **Line 14: `scanf("%[^#]", str);`**: Scans everything into `str` until user enters `#` (allows multi-line sentences).
- **Line 17: `fd1 = open("fifo1", O_WRONLY); write(fd1, str, strlen(str) + 1);`**: Sends string across pipe.

**In `6_fifo2.c`:**
- **Lines 17-26: Counting Loop**:
  - `c++`: Every character incremented.
  - `if (str[i] == '\n') l++`: Counts newlines.
  - `w++`: Triggered when current character is not space/newline and previous character was space/newline (word boundary).
- **Lines 29-38: File IO**:
  - Writes counts to `result.txt`, then reopens and reads back into `res[]`.
- **Line 41: `fd2 = open("fifo2", O_WRONLY); write(fd2, res, strlen(res) + 1);`**: Sends results back.

---

### 3. Step-by-Step Dry Run
- User in Process 1 types: `Hello World\nOS#`
- `str = "Hello World\nOS"`
- Process 2 reads `str`:
  - Characters: 14
  - Words: 3 ("Hello", "World", "OS")
  - Lines: 2 (`\n` after World, plus final line `OS`)
- Process 2 writes to `result.txt`:
  ```
  Characters: 14
  Words: 3
  Lines: 2
  ```
- Process 2 reads `result.txt` and transmits to `fifo2`.
- Process 1 displays the output on standard output.

---

### 4. Memory Blueprint
1. Both files call `mkfifo("fifo1", 0666); mkfifo("fifo2", 0666);`.
2. `fifo1.c`: `scanf("%[^#]", str)` $\to$ write `fifo1` $\to$ read `fifo2` $\to$ printf.
3. `fifo2.c`: read `fifo1` $\to$ count loop $\to$ `fopen("result.txt", "w")` $\to$ `fopen("result.txt", "r")` $\to$ write `fifo2`.

---

### 5. Viva Questions
- **Q: What is the difference between an ordinary pipe and a FIFO?**  
  *A: An ordinary pipe is anonymous and can only be used by related processes (parent-child). A FIFO has a name in the file system and can be used by any independent processes.*
- **Q: What happens if `open("fifo1", O_RDONLY)` is called before the writer starts?**  
  *A: The `open()` call blocks until a process opens `fifo1` for writing.*

---
---

# Assignment 7: Interprocess Communication — Full Duplex Pipes (CO5)

### 1. Theory & Core Concepts
- **Ordinary Pipes (`pipe()`)**: Anonymous IPC mechanism stored in kernel memory.
- **Descriptor Array**:
  - `int p[2];`
  - `p[0]`: Read end of the pipe.
  - `p[1]`: Write end of the pipe.
- **Why Two Pipes?**
  - Ordinary pipes are unidirectional.
  - To communicate both ways between parent and child:
    - Pipe 1 (`p1`): Parent writes pathname $\to$ Child reads pathname.
    - Pipe 2 (`p2`): Child writes file contents $\to$ Parent reads file contents.
- **Descriptor Hygiene**: Always close unused pipe ends! If the read end stays open in the child, `read()` will never detect EOF.

---

### 2. Code Walkthrough (`7_pipes.c`)

#### Level 1: Architecture Flow
```
pipe(p1), pipe(p2) -> fork()
├── Parent (fork() > 0):
│   Close p1[0] (read) & p2[1] (write)
│   Write filename to p1[1]
│   Read file contents from p2[0]
│   Print contents & wait(NULL)
└── Child (fork() == 0):
    Close p1[1] (write) & p2[0] (read)
    Read filename from p1[0]
    Open file with fopen() & fread()
    Write file contents to p2[1]
```

#### Level 2: Line-by-Line Breakdown
- **Line 11: `pipe(p1); pipe(p2);`**: Allocates two pipe pairs in the kernel.
- **Line 14: `if (fork() > 0)`**: Splits execution into Parent and Child.
- **Line 16: `close(p1[0]); close(p2[1]);`**: Parent closes unused read end of Pipe 1 and write end of Pipe 2.
- **Line 20: `write(p1[1], fname, strlen(fname) + 1);`**: Parent sends file path.
- **Line 22: `read(p2[0], buf, sizeof(buf));`**: Parent blocks until Child sends the file content.
- **Line 28: `close(p1[1]); close(p2[0]);`**: Child closes unused ends.
- **Line 31: `read(p1[0], fname, sizeof(fname));`**: Child reads requested path.
- **Line 33: `fp = fopen(fname, "r");`**: Child reads local disk file.
- **Line 39: `write(p2[1], buf, strlen(buf) + 1);`**: Child transmits file content back.

---

### 3. Step-by-Step Dry Run
1. Create file `test.txt` with content `"Hello OS"`.
2. Run program $\implies$ `pipe(p1)` and `pipe(p2)` created.
3. `fork()` called.
4. Parent prompts: `Enter file name: test.txt`.
5. Parent writes `"test.txt"` into `p1[1]`.
6. Child reads `"test.txt"` from `p1[0]`.
7. Child opens `test.txt`, reads `"Hello OS"`, and writes into `p2[1]`.
8. Parent reads `"Hello OS"` from `p2[0]` and prints:
   ```
   File contents received by parent:
   Hello OS
   ```
9. Parent reaps child with `wait(NULL)` and exits.

---

### 4. Memory Blueprint
1. Declare `int p1[2], p2[2]; pipe(p1); pipe(p2);`.
2. Parent (`fork() > 0`): close `p1[0], p2[1]`, write `p1[1]`, read `p2[0]`, print, `wait(NULL)`.
3. Child (`else`): close `p1[1], p2[0]`, read `p1[0]`, `fopen()`, `fread()`, write `p2[1]`.

---

### 5. Viva Questions
- **Q: Why must pipes be created BEFORE calling `fork()`?**  
  *A: A child inherits open file descriptors from its parent. If `pipe()` is called after `fork()`, the parent and child will have separate pipes and cannot communicate.*
- **Q: Which index of the pipe array is for reading, and which is for writing?**  
  *A: `0` is for reading (`p[0]`), `1` is for writing (`p[1]`). (Memory trick: standard file descriptor 0 is stdin = read, 1 is stdout = write).*

---
---

# Assignment 8: Disk Scheduling — SSTF, SCAN, C-LOOK (CO5)

### 1. Theory & Core Concepts
- **Disk Geometry**: Cylinders are numbered $0$ to $size - 1$.
  - Spindle is at the center (cylinder 0).
  - "Moving AWAY from spindle" means moving towards **HIGHER** cylinder numbers ($0 \to size - 1$).
- **The Algorithms**:
  1. **SSTF (Shortest Seek Time First)**:
     - Always serves the pending request closest to the current head position.
     - Prone to starvation of distant requests.
  2. **SCAN (Elevator Algorithm)**:
     - Head moves in the given direction (upwards) servicing requests until it hits the **disk boundary** ($size - 1$).
     - It then reverses direction and serves requests moving back down.
  3. **C-LOOK (Circular LOOK)**:
     - Head moves in the given direction (upwards) servicing requests until the **highest request** (does NOT touch boundary!).
     - It then jumps straight to the **lowest request** without servicing requests on the return trip, and continues upwards.

---

### 2. Code Walkthrough (`8_disk.c`)

#### Level 1: Architecture Flow
```
main() -> Read size, head, n, requests -> Sort req[] ascending
       -> Find 'start' (first request >= head)
       -> Call sstf()
       -> Call scan(start)
       -> Call clook(start)
```

#### Level 2: Line-by-Line Breakdown
- **Lines 8-26: `void sstf()`**:
  - Uses `done[30]` array to mark served requests.
  - Finds `min = abs(req[j] - pos)`. Adds to `total`. Sets `pos = req[idx]`.
- **Lines 28-44: `void scan(int start)`**:
  - Loop 1: `start` to `n - 1` (services higher requests).
  - Line 36: `total += (size - 1) - pos; pos = size - 1;` (travels all the way to disk boundary!).
  - Loop 2: `start - 1` down to `0` (services remaining lower requests).
- **Lines 46-59: `void clook(int start)`**:
  - Loop 1: `start` to `n - 1` (services higher requests up to max request).
  - Loop 2: `0` to `start - 1` (jumps to lowest request, then moves upwards).
- **Line 77: `while (start < n && req[start] < head) start++;`**: Finds partition point between requests below and above initial head.

---

### 3. Step-by-Step Dry Run
Given: `size = 200`, `head = 50`, `requests = [14, 37, 65, 67, 98, 122, 124, 183]` (sorted).  
`start = 2` (points to 65, because $65 \ge 50$).

#### SSTF:
- $50 \to 65 \to 67 \to 37 \to 14 \to 98 \to 122 \to 124 \to 183$.
- Total movement: $15 + 2 + 30 + 23 + 84 + 24 + 2 + 59 = 236$.

#### SCAN (away from spindle $\implies$ towards 199):
1. Upwards: $50 \to 65 \to 67 \to 98 \to 122 \to 124 \to 183$.
2. To Boundary: $183 \to 199$.
3. Reverse: $199 \to 37 \to 14$.
- Total movement: $(199 - 50) + (199 - 14) = 149 + 185 = 334$.

#### C-LOOK (away from spindle $\implies$ towards 183, then jump to 14):
1. Upwards: $50 \to 65 \to 67 \to 98 \to 122 \to 124 \to 183$.
2. Jump to lowest: $183 \to 14$.
3. Upwards again: $14 \to 37$.
- Total movement: $(183 - 50) + (183 - 14) + (37 - 14) = 133 + 169 + 23 = 325$.

---

### 4. Memory Blueprint
1. Main sorts `req[]` and finds `start` where `req[start] >= head`.
2. `sstf()`: Greedy search with `done[]` array.
3. `scan(start)`: `start` to `n-1` $\to$ add boundary `(size - 1)` $\to$ `start - 1` down to 0.
4. `clook(start)`: `start` to `n-1` $\to$ jump to 0 $\to$ `0` to `start - 1`.

---

### 5. Viva Questions
- **Q: What is the main difference between SCAN and LOOK?**  
  *A: SCAN always travels to the disk boundary (e.g. cylinder 199), whereas LOOK only travels to the final request in that direction and reverses immediately.*
- **Q: Why does C-LOOK jump to the lowest request instead of reversing?**  
  *A: C-LOOK provides a more uniform waiting time. When the head reaches the end, requests at the other end have waited the longest.*

---

## Final Quick-Recall Cheat Sheet for Exam Day

| # | Assignment | Key Headers | Core Mechanism |
|---|---|---|---|
| **1** | Address Book | *bash* | `grep -v "^$name " file > temp.txt && mv temp.txt file` |
| **2** | Fork & Wait | `<unistd.h>`, `<sys/wait.h>` | Child: `exit(0)`. Parent: `wait(NULL)`. Zombie = no wait. Orphan = parent exits. |
| **3** | Producer-Consumer | `<pthread.h>`, `<semaphore.h>` | `sem_wait(&empty)` $\to$ mutex lock $\to$ `sem_post(&full)` |
| **4** | Dining Philosophers | `<pthread.h>`, `<semaphore.h>` | `sem_wait(&room)` (max 4) breaks Circular Wait |
| **5** | Banker's Algorithm | `<stdio.h>` | `need = max - alloc`. If `need <= avail`, `avail += alloc`, add to safe sequence. |
| **6** | FIFOs (IPC) | `<fcntl.h>`, `<sys/stat.h>` | `mkfifo()`, `open(O_WRONLY)`, `open(O_RDONLY)` |
| **7** | Pipes (IPC) | `<unistd.h>` | `pipe(p1)`, `pipe(p2)`. P1: Parent $\to$ Child. P2: Child $\to$ Parent. |
| **8** | Disk Scheduling | `<stdlib.h>` | Moving away from spindle = higher cylinders. SCAN goes to `size-1`; C-LOOK wraps to `req[0]`. |
