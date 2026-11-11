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
