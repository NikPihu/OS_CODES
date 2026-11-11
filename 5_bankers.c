#include <stdio.h>

int main() {
    int p, r, alloc[10][10], max[10][10], need[10][10], avail[10];
    int done[10] = {0}, seq[10], count = 0;

    printf("Enter number of processes and resources: ");
    scanf("%d %d", &p, &r);

    printf("Enter Allocation Matrix:\n");
    for (int i = 0; i < p; i++)
        for (int j = 0; j < r; j++) scanf("%d", &alloc[i][j]);

    printf("Enter Max Matrix:\n");
    for (int i = 0; i < p; i++)
        for (int j = 0; j < r; j++) scanf("%d", &max[i][j]);

    printf("Enter Available Resources:\n");
    for (int j = 0; j < r; j++) scanf("%d", &avail[j]);

    // Calculate and display Need Matrix
    printf("\nNeed Matrix:\n");
    for (int i = 0; i < p; i++) {
        for (int j = 0; j < r; j++) {
            need[i][j] = max[i][j] - alloc[i][j];
            printf("%d ", need[i][j]);
        }
        printf("\n");
    }

    // Banker's Safety Algorithm
    for (int k = 0; k < p; k++) {
        for (int i = 0; i < p; i++) {
            if (!done[i]) {
                int possible = 1;
                for (int j = 0; j < r; j++) {
                    if (need[i][j] > avail[j]) {
                        possible = 0;
                        break;
                    }
                }
                if (possible) {
                    for (int j = 0; j < r; j++) avail[j] += alloc[i][j];
                    seq[count++] = i;
                    done[i] = 1;
                }
            }
        }
    }

    if (count == p) {
        printf("\nSystem is in SAFE State.\nSafe Sequence: < ");
        for (int i = 0; i < p; i++) printf("P%d ", seq[i]);
        printf(">\n");
    } else {
        printf("\nSystem is in UNSAFE State.\n");
    }

    return 0;
}


// compile
// gcc 5_bankers.c -o 5_bankers
// ./5_bankers

//input
// 5
// 3
// 0 1 0
// 2 0 0
// 3 0 2
// 2 1 1
// 0 0 2
// 7 5 3
// 3 2 2
// 9 0 2
// 2 2 2
// 4 3 3
// 3 3 2
// 1
// 1
// 1 0 2


/*==========================================================
  THEORY - ASSIGNMENT 5
============================================================
  B.1 Deadlock avoidance
  ----------------------
  Unlike prevention (Assignment 4), avoidance doesn't forbid any of the 4 conditions. Instead, before granting resources the OS checks: *"if I grant this, can every process still finish in some order?"* If yes → grant; if no → make the process wait.

  B.2 Safe state and safe sequence
  --------------------------------
  - A safe sequence is an order <P_a, P_b, …> such that each process can get its remaining need from (currently available + what all earlier processes in the sequence will release when they finish).
  - Safe state = at least one safe sequence exists → deadlock cannot happen.
  - Unsafe state = no safe sequence → deadlock may happen (not guaranteed).

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

  B.3 Why "Banker's"?
  -------------------
  A bank never lends out its cash in a way that leaves it unable to satisfy all its customers' maximum credit limits. Resources = cash, processes = customers, Max = credit limit.

  B.4 Data structures (n = processes, m = resource types)
  -------------------------------------------------------
  | Name | Size | Meaning |
  |---|---|---|
  | Available[m] | vector | free instances of each resource |
  | Max[n][m] | matrix | maximum demand of each process |
  | Allocation[n][m] | matrix | currently held |
  | Need[n][m] | matrix | Need = Max − Allocation (still may request) |

  B.5 Safety algorithm (textbook form)
  ------------------------------------
  1. Work = Available;   Finish[i] = false for all i
  2. Find i such that  Finish[i] == false  AND  Need[i] <= Work   (for every resource)
        If no such i → go to step 4
  3. Work = Work + Allocation[i];  Finish[i] = true;  append Pi to sequence;  go to step 2
  4. If Finish[i] == true for all i → SAFE, else UNSAFE
  Why Work += Allocation[i]? We *pretend* Pi gets its Need, runs to completion, and then returns everything it holds. Its Allocation goes back to the pool.

  Complexity: O(m × n²).

  Note: Our program uses avail directly as Work and done as Finish.

  B.6 Resource-request algorithm (theory only — not in our code)
  --------------------------------------------------------------
  When Pi requests Request[i]:
  1. If Request > Need[i] → error (exceeded its max claim).
  2. If Request > Available → Pi must wait.
  3. Pretend to allocate: Available −= Request; Allocation[i] += Request; Need[i] −= Request.
  4. Run the safety algorithm. If safe → grant. If unsafe → roll back, Pi waits.
==========================================================*/


/*==========================================================
  FUNCTIONS & SYSTEM CALLS - ASSIGNMENT 5
============================================================
  | Function | Header | Purpose |
  |---|---|---|
  | printf | <stdio.h> | Prompts, Need matrix, result |
  | scanf | <stdio.h> | Read p, r, matrices, vector |

  The whole algorithm lives in main(); there are no other functions or system calls. The logic is the interesting part.
==========================================================*/


/*==========================================================
  EXECUTION FLOW (CALL CHAIN) - ASSIGNMENT 5
============================================================
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

  Why p rounds? Each round marks at least one more process done if the state is safe. In the worst case one per round → p rounds are always enough.
==========================================================*/


/*==========================================================
  DRY RUN - ASSIGNMENT 5
============================================================
  Input: p = 5, r = 3 (resources A B C)

  | Process | Allocation (A B C) | Max (A B C) | Need = Max − Alloc |
  |---|---|---|---|
  | P0 | 0 1 0 | 7 5 3 | 7 4 3 |
  | P1 | 2 0 0 | 3 2 2 | 1 2 2 |
  | P2 | 3 0 2 | 9 0 2 | 6 0 0 |
  | P3 | 2 1 1 | 2 2 2 | 0 1 1 |
  | P4 | 0 0 2 | 4 3 3 | 4 3 1 |

  Available = 3 3 2

  Safety algorithm trace:

  | Round k | i | Need[i] | avail before | Need ≤ avail? | avail after | seq |
  |---|---|---|---|---|---|---|
  | 0 | P0 | 7 4 3 | 3 3 2 | ✗ (7>3) | 3 3 2 | – |
  | 0 | P1 | 1 2 2 | 3 3 2 | ✓ | 3+2, 3+0, 2+0 = 5 3 2 | P1 |
  | 0 | P2 | 6 0 0 | 5 3 2 | ✗ (6>5) | 5 3 2 | P1 |
  | 0 | P3 | 0 1 1 | 5 3 2 | ✓ | 7 4 3 | P1 P3 |
  | 0 | P4 | 4 3 1 | 7 4 3 | ✓ | 7 4 5 | P1 P3 P4 |
  | 1 | P0 | 7 4 3 | 7 4 5 | ✓ | 7 5 5 | P1 P3 P4 P0 |
  | 1 | P2 | 6 0 0 | 7 5 5 | ✓ | 10 5 7 | P1 P3 P4 P0 P2 |
  | 2–4 | — | all done, nothing changes | | | | |

  count = 5 = p → SAFE. Final avail = 10 5 7 = total resources in the system (a good self-check: Available + sum of all Allocation).

  Exact output:
  Need Matrix:
  7 4 3
  1 2 2
  6 0 0
  0 1 1
  4 3 1

  System is in SAFE State.
  Safe Sequence: < P1 P3 P4 P0 P2 >

  Unsafe example: change Available to 0 0 0. No process has Need ≤ (0,0,0) except one with Need all zeros → none here → count = 0 → UNSAFE.
==========================================================*/
