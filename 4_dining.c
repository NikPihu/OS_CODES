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
