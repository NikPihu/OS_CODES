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
