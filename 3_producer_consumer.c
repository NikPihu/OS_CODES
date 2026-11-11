#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define SIZE 5

int buffer[SIZE];
int in = 0, out = 0;

pthread_mutex_t mutex;
sem_t empty, full;

void *producer(void *arg) {
    for (int i = 1; i <= 5; i++) {
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);

        buffer[in] = i * 10;
        printf("Producer produced %d at index %d\n", buffer[in], in);
        in = (in + 1) % SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&full);
        sleep(1);
    }
    return NULL;
}

void *consumer(void *arg) {
    for (int i = 1; i <= 5; i++) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);

        int item = buffer[out];
        printf("Consumer consumed %d from index %d\n", item, out);
        out = (out + 1) % SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t p, c;

    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty, 0, SIZE);
    sem_init(&full, 0, 0);

    pthread_create(&p, NULL, producer, NULL);
    pthread_create(&c, NULL, consumer, NULL);

    pthread_join(p, NULL);
    pthread_join(c, NULL);

    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}

// compile
// gcc 3_producer_consumer.c -o 3_producer_consumer -pthread
// ./3_producer_consumer

// input
// 5
// 64 25 12 22 11


/*==========================================================
  THEORY - ASSIGNMENT 3
============================================================
  B.1 Threads
  -----------
  - A thread is a "lightweight process": it runs inside a process and shares its global variables, heap and files.
  - Creating/switching threads is cheaper than processes.
  - Because threads share data, two threads changing the same variable at once can corrupt it → we need synchronization.

  B.2 Race condition — why we need synchronization
  ------------------------------------------------
  in = in + 1 is really three machine steps: *load in → add 1 → store in*. If two threads interleave:
  Thread A: load in (0)
  Thread B: load in (0)
  Thread A: store 1
  Thread B: store 1      ← one update lost; in should be 2
  The result depends on timing → race condition.

  B.3 Critical section problem
  ----------------------------
  The critical section is the code that accesses shared data (here: buffer, in, out). A correct solution must guarantee:
  1. Mutual exclusion — at most one thread in the critical section.
  2. Progress — if nobody is inside, someone who wants to enter must be allowed soon.
  3. Bounded waiting — no thread waits forever.

  B.4 Mutex
  ---------
  - A mutual exclusion lock. lock() — if free, take it; if taken, block. unlock() — release it.
  - Has ownership: only the thread that locked it should unlock it.

  B.5 Semaphore (Dijkstra)
  ------------------------
  An integer S with two atomic operations:
  wait(S)   / P / sem_wait :   while (S <= 0) block;   S = S - 1;
  signal(S) / V / sem_post :   S = S + 1;   wake one blocked thread (if any)
  - Binary semaphore: values 0/1 (like a lock).
  - Counting semaphore: any non-negative value — counts available resources.

  B.6 The bounded-buffer solution
  -------------------------------
  | Semaphore / lock | Initial | Meaning |
  |---|---|---|
  | empty | SIZE (5) | number of free slots |
  | full | 0 | number of filled slots |
  | mutex | unlocked | protects buffer, in, out |

  Producer:                       Consumer:
    wait(empty)   // free slot?     wait(full)    // item available?
    lock(mutex)                     lock(mutex)
      put item at buffer[in]          take item from buffer[out]
      in = (in+1) % SIZE              out = (out+1) % SIZE
    unlock(mutex)                   unlock(mutex)
    post(full)    // one more item  post(empty)   // one more free slot
  Invariant: empty + full == SIZE (whenever no thread is in the middle of an operation).

  B.7 Circular buffer
  -------------------
  in = where the next item is written, out = where the next item is read. % SIZE wraps the index from 4 back to 0, so the array is reused like a ring:
  index:   0    1    2    3    4
          [10] [20] [  ] [  ] [  ]
                ▲    ▲
               out   in
==========================================================*/


/*==========================================================
  FUNCTIONS & SYSTEM CALLS - ASSIGNMENT 3
============================================================
  | Function | Header | Prototype | Purpose here |
  |---|---|---|---|
  | pthread_create | <pthread.h> | int pthread_create(pthread_t *t, const pthread_attr_t *attr, void *(*fn)(void *), void *arg); | Start a thread running fn(arg). Returns 0 on success |
  | pthread_join | <pthread.h> | int pthread_join(pthread_t t, void ret); | Wait for thread t to finish (like wait for threads) |
  | pthread_mutex_init | <pthread.h> | int pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *attr); | Initialise mutex (NULL = default attributes) |
  | pthread_mutex_lock / unlock | <pthread.h> | int pthread_mutex_lock(pthread_mutex_t *m); | Enter / leave critical section |
  | pthread_mutex_destroy | <pthread.h> | int pthread_mutex_destroy(pthread_mutex_t *m); | Free mutex resources |
  | sem_init | <semaphore.h> | int sem_init(sem_t *s, int pshared, unsigned value); | Create semaphore with initial value. pshared = 0 → shared between threads of this process |
  | sem_wait | <semaphore.h> | int sem_wait(sem_t *s); | P(): decrement, block if 0 |
  | sem_post | <semaphore.h> | int sem_post(sem_t *s); | V(): increment, wake a waiter |
  | sem_destroy | <semaphore.h> | int sem_destroy(sem_t *s); | Free semaphore |
  | sleep | <unistd.h> | unsigned sleep(unsigned s); | Slow down so the interleaving is visible |

  Thread function signature: must be void *fn(void *arg) — takes one generic pointer and returns one generic pointer.
==========================================================*/


/*==========================================================
  EXECUTION FLOW (CALL CHAIN) - ASSIGNMENT 3
============================================================
  main()  [main thread]
   ├─ pthread_mutex_init(&mutex)
   ├─ sem_init(&empty, 0, 5)        empty = 5
   ├─ sem_init(&full,  0, 0)        full  = 0
   ├─ pthread_create(&p, producer) ════════╗  new thread starts producer()
   ├─ pthread_create(&c, consumer) ════════╬══════╗ new thread starts consumer()
   ├─ pthread_join(p) … blocks …           ║      ║
   │                                       ▼      ▼
   │              producer()                      consumer()
   │              loop i = 1..5:                  loop i = 1..5:
   │               ├ sem_wait(&empty)              ├ sem_wait(&full)  ← blocks while full==0
   │               ├ mutex_lock                    ├ mutex_lock
   │               ├ buffer[in]=i*10; printf       ├ item=buffer[out]; printf
   │               ├ in=(in+1)%5                   ├ out=(out+1)%5
   │               ├ mutex_unlock                  ├ mutex_unlock
   │               ├ sem_post(&full) ──wakes──►    ├ sem_post(&empty) ──wakes producer if blocked
   │               └ sleep(1)                      └ sleep(1)
   │              return NULL ✖                   return NULL ✖
   ├─ pthread_join(p) returns
   ├─ pthread_join(c) returns
   ├─ pthread_mutex_destroy, sem_destroy ×2
   └─ return 0

  Short chain: main → init(mutex, empty=5, full=0) → create(producer) ‖ create(consumer) → join(p) → join(c) → destroy → return
==========================================================*/


/*==========================================================
  DRY RUN - ASSIGNMENT 3
============================================================
  | Step | Event | empty | full | in | out | Buffer |
  |---|---|---|---|---|---|---|
  | 0 | Initial | 5 | 0 | 0 | 0 | [ _ _ _ _ _ ] |
  | 1 | Consumer: sem_wait(full) → full=0 → blocks | 5 | 0 | 0 | 0 | [ _ _ _ _ _ ] |
  | 2 | Producer: wait(empty)→4, put 10 at 0, post(full) | 4 | 1 | 1 | 0 | [10 _ _ _ _ ] |
  | 3 | Consumer wakes: full→0, take 10 from 0, post(empty) | 5 | 0 | 1 | 1 | [ _ _ _ _ _ ] |
  | 4 | Producer: put 20 at 1 | 4 | 1 | 2 | 1 | [ _ 20 _ _ _ ] |
  | 5 | Consumer: take 20 from 1 | 5 | 0 | 2 | 2 | … |
  | … | continues for 30, 40, 50 | | | | | |
  | end | Both loops done | 5 | 0 | 0 | 0 | indices wrapped back to 0 |

  Output:
  Producer produced 10 at index 0
  Consumer consumed 10 from index 0
  Producer produced 20 at index 1
  Consumer consumed 20 from index 1
  Producer produced 30 at index 2
  Consumer consumed 30 from index 2
  Producer produced 40 at index 3
  Consumer consumed 40 from index 3
  Producer produced 50 at index 4
  Consumer consumed 50 from index 4
  (Exact interleaving can vary; but the consumer can never print before the matching produce.)
==========================================================*/
