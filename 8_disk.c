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
