#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MSGSIZE 64

int main(void) {
    int p[2]; // p[0]: 읽기용, p[1]: 쓰기용
    char buf[MSGSIZE] = {0};

    if (pipe(p) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();

    if (pid == 0) {
        // 자식 프로세스: 파이프에 메시지 쓰기
        close(p[0]); // 읽기 디스크립터 닫기
        char *msg = "Hello from child pipe!";
        write(p[1], msg, strlen(msg) + 1);
        close(p[1]);
        exit(0);
    } else if (pid > 0) {
        // 부모 프로세스: 파이프에서 메시지 읽기
        close(p[1]); // 쓰기 디스크립터 닫기
        read(p[0], buf, MSGSIZE);
        printf("[부모] 파이프로 받은 메시지: %s\n", buf);
        close(p[0]);
        wait(NULL);
    }

    return 0;
}