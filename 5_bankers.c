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
