#include <stdio.h>
#include <stdlib.h>

int req[30], n, head, size;

void sstf() {
    int done[30] = {0}, pos = head, total = 0, idx = 0;
    printf("\nSSTF: %d", pos);
    for (int i = 0; i < n; i++) {
        int min = 99999;
        for (int j = 0; j < n; j++) {
            if (!done[j] && abs(req[j] - pos) < min) {
                min = abs(req[j] - pos);
                idx = j;
            }
        }
        total += min;
        pos = req[idx];
        done[idx] = 1;
        printf(" -> %d", pos);
    }
    printf("\nTotal head movement = %d\n", total);
}

void scan(int start) {
    int pos = head, total = 0;
    printf("\nSCAN: %d", pos);
    for (int i = start; i < n; i++) { // move away from spindle (upwards)
        total += req[i] - pos;
        pos = req[i];
        printf(" -> %d", pos);
    }
    total += (size - 1) - pos; // reach disk boundary
    pos = size - 1;
    printf(" -> %d", pos);
    for (int i = start - 1; i >= 0; i--) { // reverse downwards
        total += pos - req[i];
        pos = req[i];
        printf(" -> %d", pos);
    }
    printf("\nTotal head movement = %d\n", total);
}

void clook(int start) {
    int pos = head, total = 0;
    printf("\nC-LOOK: %d", pos);
    for (int i = start; i < n; i++) { // move away from spindle (upwards)
        total += req[i] - pos;
        pos = req[i];
        printf(" -> %d", pos);
    }
    for (int i = 0; i < start; i++) { // jump to lowest request, move up again
        total += abs(req[i] - pos);
        pos = req[i];
        printf(" -> %d", pos);
    }
    printf("\nTotal head movement = %d\n", total);
}

int main() {
    printf("Enter disk size: ");
    scanf("%d", &size);
    printf("Enter initial head position: ");
    scanf("%d", &head);
    printf("Enter number of requests: ");
    scanf("%d", &n);
    printf("Enter requests: ");
    for (int i = 0; i < n; i++) scanf("%d", &req[i]);

    // Sort requests
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (req[j] > req[j + 1]) {
                int t = req[j];
                req[j] = req[j + 1];
                req[j + 1] = t;
            }

    int start = 0;
    while (start < n && req[start] < head) start++;

    sstf();
    scan(start);
    clook(start);

    return 0;
}

//compile
// gcc 8_disk.c -o disk
// ./disk

//input
// 200
// 50
// 8
// 98 183 37 122 14 124 65 67




/*==========================================================
  THEORY - ASSIGNMENT 8
============================================================
  B.1 Disk structure
  ------------------
          ┌───────────── platter (spins)
     ═════╪═════  track = a ring on one surface
          │       cylinder = the same track on all surfaces (numbered 0 … size-1)
     ─────┘       sector = a slice of a track
     ▲
     read/write head on an arm that moves in/out across cylinders

  B.2 Disk access time
  --------------------
  Access time = Seek time + Rotational latency + Transfer time
  - Seek time: move the arm to the right cylinder — the largest part.
  - Rotational latency: wait for the sector to rotate under the head.
  - Transfer time: actually read/write the data.

  Disk scheduling tries to minimise total seek distance (head movement) → higher throughput and lower response time.

  B.3 The algorithms
  ------------------

  | Algorithm | Rule | Pros | Cons |
  |---|---|---|---|
  | FCFS | Serve in arrival order | Fair, simple | Lots of back-and-forth movement |
  | SSTF (Shortest Seek Time First) | Always go to the closest pending request | Much less movement than FCFS | Starvation of far requests; not optimal |
  | SCAN (Elevator) | Move in one direction serving requests until the disk end, then reverse | No starvation, uniform-ish wait | Goes to the end even if no requests there |
  | C-SCAN | Like SCAN, but after reaching the end jump to cylinder 0 and continue in the same direction | More uniform waiting time | Long return seek |
  | LOOK | Like SCAN but reverse at the last request, not the disk end | Less movement than SCAN | — |
  | C-LOOK | Like C-SCAN but only go as far as the last request, then jump to the lowest request | Least wasted movement among the "elevator" family | Jump distance still costs time |

  B.4 Visual comparison (head = 50, requests = 16 24 43 82 140 170 190, disk 0–199)
  ---------------------------------------------------------------------------------
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
==========================================================*/


/*==========================================================
  FUNCTIONS & SYSTEM CALLS - ASSIGNMENT 8
============================================================
  | Function | Kind | Purpose |
  |---|---|---|
  | main() | user | Read input, sort requests, find split point, call the 3 algorithms |
  | sstf() | user | Repeatedly pick the nearest unserved request |
  | scan(int start) | user | Go up to the disk end, then come back down |
  | clook(int start) | user | Go up to the highest request, jump to the lowest, go up again |
  | abs(x) | <stdlib.h> | Absolute value → distance between two cylinders |
  | printf / scanf | <stdio.h> | I/O |

  Global variables (shared by all functions, so no parameters needed):
  req[30] (request queue), n (count), head (initial position), size (number of cylinders).
==========================================================*/


/*==========================================================
  EXECUTION FLOW (CALL CHAIN) - ASSIGNMENT 8
============================================================
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

  Short chain: main → scanf → sort → find start → sstf → printf → main → scan(start) → printf → main → clook(start) → printf → main → return

  Why sort first? Once sorted, "requests above the head in increasing order" is just req[start..n-1] and "below the head in decreasing order" is req[start-1..0]. SCAN and C-LOOK become simple loops. (SSTF doesn't need sorting, but it doesn't hurt — it also makes ties resolve to the lower cylinder.)
==========================================================*/


/*==========================================================
  DRY RUN - ASSIGNMENT 8
============================================================
  Input: disk size = 200 (cylinders 0–199), head = 50, n = 7, requests = 82 170 43 140 24 16 190

  After sorting: req = [16, 24, 43, 82, 140, 170, 190] (indices 0–6)
  Split: 16<50 → 24<50 → 43<50 → 82≥50 stop → start = 3

  SSTF
  ----

  | Step | pos | Distances to unserved requests | Chosen | Move | Total |
  |---|---|---|---|---|---|
  | 1 | 50 | 16:34 24:26 43:7 82:32 140:90 170:120 190:140 | 43 | 7 | 7 |
  | 2 | 43 | 16:27 24:19 82:39 … | 24 | 19 | 26 |
  | 3 | 24 | 16:8 82:58 … | 16 | 8 | 34 |
  | 4 | 16 | 82:66 140:124 … | 82 | 66 | 100 |
  | 5 | 82 | 140:58 170:88 190:108 | 140 | 58 | 158 |
  | 6 | 140 | 170:30 190:50 | 170 | 30 | 188 |
  | 7 | 170 | 190:20 | 190 | 20 | 208 |

  SCAN (towards higher cylinders)
  -------------------------------

  | Move | From → To | Distance | Total |
  |---|---|---|---|
  | up | 50 → 82 | 32 | 32 |
  | up | 82 → 140 | 58 | 90 |
  | up | 140 → 170 | 30 | 120 |
  | up | 170 → 190 | 20 | 140 |
  | to end | 190 → 199 | 9 | 149 |
  | down | 199 → 43 | 156 | 305 |
  | down | 43 → 24 | 19 | 324 |
  | down | 24 → 16 | 8 | 332 |

  Shortcut formula: (size−1 − head) + (size−1 − lowest) = 149 + 183 = 332 ✓

  C-LOOK
  ------

  | Move | From → To | Distance | Total |
  |---|---|---|---|
  | up | 50 → 82 → 140 → 170 → 190 | 140 | 140 |
  | jump | 190 → 16 | 174 | 314 |
  | up | 16 → 24 | 8 | 322 |
  | up | 24 → 43 | 19 | 341 |

  Exact output:
  SSTF: 50 -> 43 -> 24 -> 16 -> 82 -> 140 -> 170 -> 190
  Total head movement = 208

  SCAN: 50 -> 82 -> 140 -> 170 -> 190 -> 199 -> 43 -> 24 -> 16
  Total head movement = 332

  C-LOOK: 50 -> 82 -> 140 -> 170 -> 190 -> 16 -> 24 -> 43
  Total head movement = 341

  | Algorithm | Total movement | Observation |
  |---|---|---|
  | SSTF | 208 (best here) | Greedy, but 190 waited until the end — starvation risk |
  | SCAN | 332 | Wasted 9 cylinders going to 199 with no request there |
  | C-LOOK | 341 | Big jump counted; gives most uniform waiting time |
==========================================================*/
