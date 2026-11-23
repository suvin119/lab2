#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFSIZE 128

int main(void) {
    int p2c[2]; // 부모 -> 자식 파이프 (p2c[0]: 읽기, p2c[1]: 쓰기)
    int c2p[2]; // 자식 -> 부모 파이프 (c2p[0]: 읽기, c2p[1]: 쓰기)

    // 파이프 2개 생성
    if (pipe(p2c) == -1 || pipe(c2p) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // [자식]
        close(p2c[1]); // 부모->자식 파이프의 쓰기 끝 닫기
        close(c2p[0]); // 자식->부모 파이프의 읽기 끝 닫기

        char buf[BUFSIZE];

        while (1) {
            // 1. 부모가 보낸 메시지 읽기
            ssize_t n = read(p2c[0], buf, BUFSIZE);
            if (n <= 0) break;

            if (strcmp(buf, "quit") == 0) {
                printf("[자식] 부모가 종료\n");
                break;
            }

            printf("[자식 수신] %s\n", buf);

            // 2. 자식이 키보드로 답장 입력
            printf("[자식 입력] > ");
            fflush(stdout);
            if (fgets(buf, BUFSIZE, stdin) == NULL) break;
            buf[strcspn(buf, "\n")] = '\0';

            write(c2p[1], buf, strlen(buf) + 1);

            if (strcmp(buf, "quit") == 0) {
                printf("[자식] 채팅 종료.\n");
                break;
            }
        }

        close(p2c[0]);
        close(c2p[1]);
        exit(0);
    } else {
        // [부모]
        close(p2c[0]); // 부모->자식 파이프의 읽기 끝 닫기
        close(c2p[1]); // 자식->부모 파이프의 쓰기 끝 닫기

        char buf[BUFSIZE];

        while (1) {
            // 1. 부모가 키보드로 입력 후 자식에게 쓰기
            printf("[부모 입력] > ");
            fflush(stdout);
            if (fgets(buf, BUFSIZE, stdin) == NULL) break;
            buf[strcspn(buf, "\n")] = '\0';

            write(p2c[1], buf, strlen(buf) + 1);

            if (strcmp(buf, "quit") == 0) {
                break;
            }

            // 2. 자식이 보낸 답장 읽기
            ssize_t n = read(c2p[0], buf, BUFSIZE);
            if (n <= 0) break;

            if (strcmp(buf, "quit") == 0) {
                printf("[부모] 자식이 종료\n");
                break;
            }

            printf("[부모 수신] %s\n\n", buf);
        }

        close(p2c[1]);
        close(c2p[0]);
        wait(NULL);
        printf("[부모] 종료\n");
    }

    return 0;
}