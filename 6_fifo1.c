#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    char str[500], res[500];
    int fd1, fd2;

    mkfifo("fifo1", 0666);
    mkfifo("fifo2", 0666);

    printf("Enter sentences (end with #):\n");
    scanf("%[^#]", str);

    // Write sentences to fifo1
    fd1 = open("fifo1", O_WRONLY);
    write(fd1, str, strlen(str) + 1);
    close(fd1);

    // Read result back from fifo2
    fd2 = open("fifo2", O_RDONLY);
    read(fd2, res, sizeof(res));
    close(fd2);

    printf("\nResult from second process:\n%s\n", res);
    return 0;
}
