# Corrected OS Laboratory Programs

This directory contains the verified, corrected, and syllabus-aligned C programs and Shell scripts for the Operating Systems Laboratory course.

---

## Syllabus Mapping & Summary of Programs

| File | Assignment Title | Course Outcome | Description |
|---|---|---|---|
| [`1_address_book.sh`](./1_address_book.sh) | Shell Programming | **CO1** | Address book with menu options: Create, View, Insert, Delete, Modify, Exit. Supports both letter (`a-f`) and numeric (`1-6`) choices with validation. |
| [`2_process.c`](./2_process.c) | Process Management | **CO2** | Demonstration of `fork()` and `wait()`. Parent sorts using Bubble Sort; Child sorts using Insertion Sort and prints sorted array. Demonstrates Zombie and Orphan states. |
| [`3_producer_consumer.c`](./3_producer_consumer.c) | Thread Synchronization | **CO3** | Classical Producer-Consumer problem using counting semaphores (`empty`, `full`) and mutual exclusion mutex (`pthread_mutex_t`). |
| [`4_dining.c`](./4_dining.c) | Thread Sync & Deadlock | **CO3, CO4** | Deadlock-free solution to Dining Philosophers problem using Dijkstra's room semaphore (allows max 4 philosophers at table). |
| [`5_bankers.c`](./5_bankers.c) | Deadlock Avoidance | **CO4** | Complete Banker's Algorithm: prints the computed Need Matrix, verifies Safety state, and handles optional Resource Request testing. |
| [`6_fifo1.c`](./6_fifo1.c)<br>[`6_fifo2.c`](./6_fifo2.c) | Interprocess Communication (FIFOs) | **CO5** | Full-duplex IPC using named pipes (`fifo1`, `fifo2`). Process 1 sends sentences; Process 2 counts characters, words, and lines, writes to `result.txt`, and returns summary via `fifo2`. |
| [`7_pipes.c`](./7_pipes.c) | Interprocess Communication (Pipes) | **CO5** | Full-duplex IPC using ordinary pipes. Parent sends file path via Pipe 1; Child reads path, opens file, and transmits contents back via Pipe 2 for Parent to display. |
| [`8_disk.c`](./8_disk.c) | Disk Scheduling | **CO5** | Simulates SSTF, SCAN, and C-LOOK disk scheduling algorithms with initial head moving away from spindle (towards higher cylinders). |

---

## Detailed Compilation, Execution, & Sample Inputs

### 1. Address Book (`1_address_book.sh`)
```bash
chmod +x 1_address_book.sh
./1_address_book.sh
```
#### Interactive Example Run:
1. **Create Address Book**: Choose `a` or `1`.
2. **Insert Record 1**: Choose `c` or `3`
   ```
   Enter Name: Alice
   Enter Phone: 9876543210
   Enter City: Pune
   ```
3. **Insert Record 2**: Choose `c` or `3`
   ```
   Enter Name: Bob
   Enter Phone: 9123456780
   Enter City: Mumbai
   ```
4. **View Address Book**: Choose `b` or `2`
   ```
   ------------------------------------------
   NAME            PHONE           CITY           
   ------------------------------------------
   Alice           9876543210      Pune           
   Bob             9123456780      Mumbai         
   ------------------------------------------
   ```
5. **Modify Record**: Choose `e` or `5`
   ```
   Enter name to modify: Alice
   Enter New Phone: 9998887776
   Enter New City: Delhi
   ```
6. **Delete Record**: Choose `d` or `4`
   ```
   Enter name to delete: Bob
   ```
7. **Exit**: Choose `f` or `6`.

---

### 2. Process Management (`2_process.c`)
```bash
gcc 2_process.c -o 2_process
./2_process
```
#### Sample Input:
```
5
64 25 12 22 11
```
#### Expected Output Highlights:
- **Child Process**: Sorts using Insertion Sort $\rightarrow$ `11 12 22 25 64`
- **Parent Process**: Sorts using Bubble Sort $\rightarrow$ `11 12 22 25 64`
- **Parent `wait()`**: Parent waits for Child PID and resumes after termination.
- **Zombie State Demo**: Child PID terminates immediately; Parent sleeps 4s without `wait()`. Child shows as `[2_process] <defunct>` in process table. Parent then reaps with `waitpid()`.
- **Orphan State Demo**: Parent terminates immediately; Child sleeps 3s. Child wakes up and prints `New Parent PID: 1` (or `systemd`).

---

### 3. Producer-Consumer Problem (`3_producer_consumer.c`)
```bash
gcc 3_producer_consumer.c -o 3_producer_consumer -pthread
./3_producer_consumer
```
#### Input:
- Fully automated demonstration (no input needed).
#### Expected Output:
- Producers 1 & 2 insert items into circular buffer slots $0..4$.
- Consumers 1 & 2 remove items from buffer.
- Mutex ensures mutual exclusion while counting semaphores `empty` and `full` prevent buffer overflow and underflow.

---

### 4. Dining Philosophers Problem (`4_dining.c`)
```bash
gcc 4_dining.c -o 4_dining -pthread
./4_dining
```
#### Input:
- Fully automated demonstration (no input needed).
#### Expected Output:
- 5 philosophers think and eat over 2 cycles without deadlock.
- Room semaphore (`sem_t room = 4`) ensures at most 4 philosophers can compete for forks simultaneously, breaking the circular wait condition.

---

### 5. Banker's Algorithm (`5_bankers.c`)
```bash
gcc 5_bankers.c -o 5_bankers
./5_bankers
```
#### Standard Benchmark Sample Input:
```
5
3
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
1
1
1 0 2
```
#### Input Breakdown:
- Number of processes: `5`
- Number of resource types: `3`
- Allocation Matrix:
  - P0: `0 1 0`
  - P1: `2 0 0`
  - P2: `3 0 2`
  - P3: `2 1 1`
  - P4: `0 0 2`
- Max Matrix:
  - P0: `7 5 3`
  - P1: `3 2 2`
  - P2: `9 0 2`
  - P3: `2 2 2`
  - P4: `4 3 3`
- Available Resources: `3 3 2`
- Test Resource Request? `1` (Yes)
  - Process ID: `1`
  - Request: `1 0 2`

#### Expected Output:
- Displays computed **Need Matrix**:
  - P0: `7 4 3`
  - P1: `1 2 2`
  - P2: `6 0 0`
  - P3: `0 1 1`
  - P4: `4 3 1`
- Safe State: `System is currently in a SAFE STATE.`
- Safe Sequence: `< P1 P3 P4 P0 P2 >`
- Resource Request: `[SUCCESS]: Request can be GRANTED immediately!`

---

### 6. Full Duplex FIFOs (`6_fifo1.c` & `6_fifo2.c`)
Run in two separate terminals in the same folder:
```bash
# Terminal 1:
gcc 6_fifo2.c -o fifo2
./fifo2

# Terminal 2:
gcc 6_fifo1.c -o fifo1
./fifo1
```

#### Sample Input in Terminal 2 (Process 1):
```
Operating Systems is fun.
Pipes and FIFOs enable IPC.
Process synchronization is critical.#
```
*(Note: end input with `#`)*

#### Expected Output in Terminal 1 (Process 2):
```
[Process 2] Analysis results:
  Characters : 88
  Words      : 11
  Lines      : 3
[Process 2] Output written to 'result.txt'.
[Process 2] File contents successfully sent to Process 1 via 'fifo2'.
```

#### Expected Output in Terminal 2 (Process 1):
```
OUTPUT RECEIVED FROM PROCESS 2 (via fifo2):
--- Text Analysis Summary ---
Number of Characters : 88
Number of Words      : 11
Number of Lines      : 3

[Process 1] Named pipes unlinked and cleaned up.
```

---

### 7. Full Duplex Pipes (`7_pipes.c`)
```bash
# Create a test file first:
echo "Operating Systems Lab: Full duplex IPC using ordinary pipes." > testfile.txt

# Compile and run:
gcc 7_pipes.c -o pipes
./pipes
```
#### Sample Input:
```
testfile.txt
```
#### Expected Output:
```
[Parent] Writing file path 'testfile.txt' to Pipe 1...
[Parent] Waiting to receive file contents from Child via Pipe 2...

====================================================
  FILE CONTENTS DISPLAYED BY PARENT (from Child):
====================================================
Operating Systems Lab: Full duplex IPC using ordinary pipes.

====================================================
[Parent] Child process terminated. Parent exiting.
```

---

### 8. Disk Scheduling (`8_disk.c`)
```bash
gcc 8_disk.c -o disk
./disk
```
#### Standard Textbook Sample Input:
```
200
50
8
98 183 37 122 14 124 65 67
```
#### Input Breakdown:
- Disk size: `200` (cylinders 0 to 199)
- Initial head position: `50`
- Number of requests: `8`
- Requests: `98 183 37 122 14 124 65 67`

#### Expected Output:
- **1. SSTF**:
  - Seek Sequence: `50 -> 65 -> 67 -> 37 -> 14 -> 98 -> 122 -> 124 -> 183`
  - Total Head Movement: `236 cylinders`
  - Average Seek Length: `29.50 cylinders`
- **2. SCAN** (Direction: away from spindle $\implies$ towards 199):
  - Seek Sequence: `50 -> 65 -> 67 -> 98 -> 122 -> 124 -> 183 -> 199 (Boundary) -> 37 -> 14`
  - Total Head Movement: `334 cylinders`
  - Average Seek Length: `41.75 cylinders`
- **3. C-LOOK** (Direction: away from spindle $\implies$ towards 183, then circular jump to 14):
  - Seek Sequence: `50 -> 65 -> 67 -> 98 -> 122 -> 124 -> 183 -> [Jump: 14] -> 37`
  - Total Head Movement (including return jump): `325 cylinders`
  - Seek Movement (servicing requests only): `156 cylinders`
  - Average Seek Length (with jump): `40.62 cylinders`

---

## Key Theoretical Concepts for Practical Exams / Viva

- **Zombie Process**: A terminated child process whose exit status has not yet been read by its parent via `wait()`. Exists as `<defunct>` in the process table.
- **Orphan Process**: A running child process whose parent has terminated. Adopted by `init` (PID 1) or `systemd`.
- **Counting Semaphore vs Mutex**: Mutex provides mutual exclusion (locking a single critical resource, binary ownership). Counting semaphore manages access to a finite pool of identical resources (e.g., buffer slots).
- **Dining Philosophers Deadlock Avoidance**: Limiting the number of seated philosophers to $N - 1 = 4$ ensures at least one philosopher can acquire two forks simultaneously by the Pigeonhole Principle, preventing circular wait.
- **Banker's Algorithm**:
  - $Need[i][j] = Max[i][j] - Allocation[i][j]$
  - Safety check: tests whether a safe sequence of processes exists where each process can satisfy its remaining need using currently available resources.
  - Resource Request: simulates temporary allocation only if $Request \le Need$ and $Request \le Available$, and commits it only if the resulting state is safe.
- **Head Moving Away from Spindle**: The spindle is at the center of disk platters (track 0). Moving away from the spindle means moving outwards towards higher cylinder numbers ($0 \to \text{size} - 1$).
