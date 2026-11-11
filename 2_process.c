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
