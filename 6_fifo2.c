#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    char str[500], res[500];
    int fd1, fd2, c = 0, w = 0, l = 0;
    FILE *fp;

    mkfifo("fifo1", 0666);
    mkfifo("fifo2", 0666);

    // Read sentences from fifo1
    fd1 = open("fifo1", O_RDONLY);
    read(fd1, str, sizeof(str));
    close(fd1);

    // Count characters, words, lines
    for (int i = 0; str[i] != '\0'; i++) {
        c++;
        if (str[i] == '\n') l++;
        if (str[i] != ' ' && str[i] != '\n' && str[i] != '\t' &&
            (i == 0 || str[i - 1] == ' ' || str[i - 1] == '\n' || str[i - 1] == '\t'))
            w++;
    }
    if (c > 0 && str[c - 1] != '\n') l++;

    // Write output to file
    fp = fopen("result.txt", "w");
    fprintf(fp, "Characters: %d\nWords: %d\nLines: %d\n", c, w, l);
    fclose(fp);

    // Read output from file
    fp = fopen("result.txt", "r");
    int n = fread(res, 1, sizeof(res) - 1, fp);
    res[n] = '\0';
    fclose(fp);

    // Write file contents to fifo2
    fd2 = open("fifo2", O_WRONLY);
    write(fd2, res, strlen(res) + 1);
    close(fd2);

    printf("Analysis written to result.txt and sent back via fifo2.\n");
    return 0;
}
