#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int p1[2], p2[2], n;
    char fname[100], buf[1000];
    FILE *fp;

    pipe(p1); // p1: parent -> child (file name)
    pipe(p2); // p2: child -> parent (file content)

    if (fork() > 0) {
        // Parent Process
        close(p1[0]);
        close(p2[1]);

        printf("Enter file name: ");
        scanf("%s", fname);
        write(p1[1], fname, strlen(fname) + 1);

        read(p2[0], buf, sizeof(buf));
        printf("\nFile contents received by parent:\n%s\n", buf);
        wait(NULL);
    } else {
        // Child Process
        close(p1[1]);
        close(p2[0]);

        read(p1[0], fname, sizeof(fname));

        fp = fopen(fname, "r");
        if (fp == NULL) {
            strcpy(buf, "File not found!");
        } else {
            n = fread(buf, 1, sizeof(buf) - 1, fp);
            buf[n] = '\0';
            fclose(fp);
        }
        write(p2[1], buf, strlen(buf) + 1);
    }
    return 0;
}
