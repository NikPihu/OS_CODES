#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t fork_s[5]; // one semaphore per fork
sem_t room;      // allows max 4 philosophers at a time (prevents deadlock)

void *philosopher(void *arg) {
    int i = *(int *)arg;

    printf("Philosopher %d is thinking\n", i);
    sem_wait(&room); // enter room

    sem_wait(&fork_s[i]);           // pick left fork
    sem_wait(&fork_s[(i + 1) % 5]); // pick right fork

    printf("--> Philosopher %d is eating\n", i);
    sleep(1);

    sem_post(&fork_s[(i + 1) % 5]); // put right fork
    sem_post(&fork_s[i]);           // put left fork

    sem_post(&room); // leave room
    printf("Philosopher %d finished eating\n", i);
    return NULL;
}

int main() {
    pthread_t t[5];
    int id[5];

    sem_init(&room, 0, 4);
    for (int i = 0; i < 5; i++) sem_init(&fork_s[i], 0, 1);

    for (int i = 0; i < 5; i++) {
        id[i] = i;
        pthread_create(&t[i], NULL, philosopher, &id[i]);
    }
    for (int i = 0; i < 5; i++) pthread_join(t[i], NULL);

    sem_destroy(&room);
    for (int i = 0; i < 5; i++) sem_destroy(&fork_s[i]);

    return 0;
}

//compile
// gcc 4_dining.c -o 4_dining -pthread
// ./4_dining


/*==========================================================
  THEORY - ASSIGNMENT 4
============================================================
  B.1 The problem
  ---------------
                P0
           F0        F1
        P4              P1
           F4        F2
            P3   F3   P2

   Philosopher i needs: left fork = F[i], right fork = F[(i+1) % 5]
     P0: F0,F1   P1: F1,F2   P2: F2,F3   P3: F3,F4   P4: F4,F0
  It models processes competing for a limited set of shared resources.

  B.2 What is deadlock?
  ---------------------
  A set of processes where each is waiting for a resource held by another in the set, so none can ever continue.

  B.3 Four necessary conditions (Coffman) — all must hold
  -------------------------------------------------------
  | Condition | In the dining philosophers |
  |---|---|
  | Mutual exclusion | A fork can be held by only one philosopher |
  | Hold and wait | A philosopher holds the left fork while waiting for the right |
  | No preemption | You can't snatch a fork from someone |
  | Circular wait | P0 waits for P1's fork, P1 for P2's, … P4 for P0's |

  Naive deadlock: all 5 pick up their left fork at the same instant → every right fork is taken → all wait forever.

  B.4 Ways to handle deadlock
  ---------------------------
  | Strategy | Idea |
  |---|---|
  | Prevention | Make one of the four conditions impossible |
  | Avoidance | Check every request and only grant it if the state stays *safe* (Banker's — Assignment 5) |
  | Detection & recovery | Let it happen, detect the cycle, kill/rollback |
  | Ignore (Ostrich) | Assume it's rare (what most OSs do) |

  B.5 Our solution: the "room" (footman) semaphore
  ------------------------------------------------
  - A counting semaphore room = 4 (N − 1). A philosopher must enter the room before touching forks.
  - At most 4 philosophers compete for 5 forks. By the pigeonhole principle at least one of them gets both forks, eats, and releases them → progress always continues.
  - This breaks circular wait (a cycle needs all 5).

  Other known solutions (good viva answers):
  1. Asymmetric: odd philosophers pick left first, even pick right first.
  2. Pick both or none (check both forks inside a critical section).
  3. Resource ordering: always pick the lower-numbered fork first.
  4. Monitor solution with states THINKING/HUNGRY/EATING.

  B.6 Starvation vs Deadlock
  --------------------------
  - Deadlock: nobody progresses.
  - Starvation: the system progresses but one particular thread may wait indefinitely. (In our program each philosopher eats only once, so starvation isn't an issue.)
==========================================================*/


/*==========================================================
  FUNCTIONS & SYSTEM CALLS - ASSIGNMENT 4
============================================================
  | Function | Header | Purpose here |
  |---|---|---|
  | sem_init(&s, 0, v) | <semaphore.h> | room = 4, each fork_s[i] = 1 (binary semaphore = fork free) |
  | sem_wait(&s) | <semaphore.h> | Enter room / pick up a fork (blocks if unavailable) |
  | sem_post(&s) | <semaphore.h> | Put a fork down / leave room |
  | sem_destroy(&s) | <semaphore.h> | Clean up |
  | pthread_create(&t, NULL, fn, arg) | <pthread.h> | One thread per philosopher, arg = pointer to its ID |
  | pthread_join(t, NULL) | <pthread.h> | Main waits for all philosophers |
  | sleep(1) | <unistd.h> | Simulate eating time |
==========================================================*/


/*==========================================================
  EXECUTION FLOW (CALL CHAIN) - ASSIGNMENT 4
============================================================
  main()
   ├─ sem_init(&room, 0, 4)
   ├─ for i=0..4: sem_init(&fork_s[i], 0, 1)          all forks free
   ├─ for i=0..4: id[i]=i; pthread_create(&t[i], philosopher, &id[i])
   │        ╠══► philosopher(&id[0])
   │        ╠══► philosopher(&id[1])
   │        ╠══► philosopher(&id[2])     5 threads run concurrently
   │        ╠══► philosopher(&id[3])
   │        ╚══► philosopher(&id[4])
   ├─ for i=0..4: pthread_join(t[i])  … waits …
   ├─ sem_destroy(room), sem_destroy(fork_s[0..4])
   └─ return 0

  philosopher(arg)          [each thread]
   ├─ i = *(int*)arg
   ├─ printf "thinking"
   ├─ sem_wait(&room)               ← 5th philosopher blocks here until someone leaves
   ├─ sem_wait(&fork_s[i])          ← left fork
   ├─ sem_wait(&fork_s[(i+1)%5])    ← right fork
   ├─ printf "eating"; sleep(1)
   ├─ sem_post(&fork_s[(i+1)%5])    ← put right fork
   ├─ sem_post(&fork_s[i])          ← put left fork
   ├─ sem_post(&room)               ← leave room, lets a waiting philosopher in
   ├─ printf "finished eating"
   └─ return NULL

  Short chain: main → init(room=4, forks=1) → create ×5 → philosopher: think → room↓ → left↓ → right↓ → eat → right↑ → left↑ → room↑ → main: join ×5 → destroy
==========================================================*/


/*==========================================================
  DRY RUN - ASSIGNMENT 4
============================================================
  Assume all threads start almost together and P0–P3 get into the room first.

  | Time | Event | room | Forks held (F0..F4) |
  |---|---|---|---|
  | t0 | P0,P1,P2,P3 enter room; P4 blocks on room | 0 | — |
  | t0 | P0 takes F0, F1 → eats | 0 | F0:P0 F1:P0 |
  | t0 | P1 takes … F1 busy → blocks on F1 | 0 | |
  | t0 | P2 takes F2, F3 → eats | 0 | F2:P2 F3:P2 |
  | t0 | P3 takes … F3 busy → blocks on F3 | 0 | |
  | t1 | P0 puts F1, F0, leaves room → room 1; P4 enters (room 0) | 0 | |
  | t1 | P1 gets F1, then F2 (after P2 releases) → eats | | |
  | t1 | P4 takes F4, F0 → eats | | |
  | … | P3 eventually gets F3, F4 → eats | | |
  | end | Everyone has eaten once; all threads joined | 4 | all free |

  Sample output (order varies each run):
  Philosopher 0 is thinking
  Philosopher 1 is thinking
  Philosopher 2 is thinking
  Philosopher 3 is thinking
  Philosopher 4 is thinking
  --> Philosopher 0 is eating
  --> Philosopher 2 is eating
  Philosopher 0 finished eating
  Philosopher 2 finished eating
  --> Philosopher 4 is eating
  --> Philosopher 1 is eating
  Philosopher 4 finished eating
  Philosopher 1 finished eating
  --> Philosopher 3 is eating
  Philosopher 3 finished eating
  Key observation: two *neighbours* are never "eating" at the same time (they share a fork), but non-neighbours (e.g. 0 and 2) can eat together.
==========================================================*/
