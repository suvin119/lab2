#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/wait.h>

int main(void) {
    int fd = open("lock_test.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    pid_t pid = fork();

    if (pid == 0) {
        // 자식 프로세스: 잠금 후 쓰기
        flock(fd, LOCK_EX);
        printf("[자식] 파일 잠금\n");
        write(fd, "Child data\n", 11);
        sleep(1);
        printf("[자식] 파일 잠금 해제\n");
        flock(fd, LOCK_UN);
        close(fd);
        exit(0);
    } else if (pid > 0) {
        // 부모 프로세스: 잠금 후 쓰기
        flock(fd, LOCK_EX);
        printf("[부모] 파일 잠금\n");
        write(fd, "Parent data\n", 12);
        sleep(1);
        printf("[부모] 파일 잠금 해제\n");
        flock(fd, LOCK_UN);

        wait(NULL);
        close(fd);
    }

    return 0;
}