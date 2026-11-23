#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>

#define Q_P2C "/q_p2c" // 부모 -> 자식 메시지 큐 이름
#define Q_C2P "/q_c2p" // 자식 -> 부모 메시지 큐 이름
#define BUFSIZE 128

int main(void) {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10; // 최대 메시지 개수
    attr.mq_msgsize = BUFSIZE; // 메시지 최대 바이트 크기
    attr.mq_curmsgs = 0;

    // 이전 실행에서 남은 큐가 있다면 미리 제거
    mq_unlink(Q_P2C);
    mq_unlink(Q_C2P);

    // POSIX 메시지 큐 2개 생성
    mqd_t q_p2c = mq_open(Q_P2C, O_CREAT | O_RDWR, 0666, &attr);
    mqd_t q_c2p = mq_open(Q_C2P, O_CREAT | O_RDWR, 0666, &attr);
    if (q_p2c == (mqd_t)-1 || q_c2p == (mqd_t)-1) {
        perror("mq_open");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // [자식]
        char buf[BUFSIZE];
        unsigned int prio;

        while (1) {
            // 1. 부모가 보낸 메시지 수신 (q_p2c)
            ssize_t n = mq_receive(q_p2c, buf, BUFSIZE, &prio);
            if (n == -1) {
                perror("mq_receive");
                exit(1);
            }
            buf[n] = '\0';

            if (strcmp(buf, "quit") == 0) {
                printf("[자식] 부모가 채팅 종료\n");
                break;
            }

            printf("[자식 수신] %s\n", buf);

            // 2. 자식이 입력 후 부모에게 전송 (q_c2p)
            printf("[자식 입력] > ");
            fflush(stdout);
            if (fgets(buf, BUFSIZE, stdin) == NULL) break;
            buf[strcspn(buf, "\n")] = '\0';

            if (mq_send(q_c2p, buf, strlen(buf) + 1, 1) == -1) {
                perror("mq_send");
                exit(1);
            }

            if (strcmp(buf, "quit") == 0) {
                printf("[자식] 채팅 종료\n");
                break;
            }
        }

        mq_close(q_p2c);
        mq_close(q_c2p);
        exit(0);
    } else {
        // [부모]
        char buf[BUFSIZE];
        unsigned int prio;

        while (1) {
            // 1. 부모가 키보드로 입력 후 자식에게 전송 (q_p2c)
            printf("[부모 입력] > ");
            fflush(stdout);
            if (fgets(buf, BUFSIZE, stdin) == NULL) break;
            buf[strcspn(buf, "\n")] = '\0';

            if (mq_send(q_p2c, buf, strlen(buf) + 1, 1) == -1) {
                perror("mq_send");
                exit(1);
            }

            if (strcmp(buf, "quit") == 0) {
                break;
            }

            // 2. 자식이 보낸 답장 수신 (q_c2p)
            ssize_t n = mq_receive(q_c2p, buf, BUFSIZE, &prio);
            if (n == -1) {
                perror("mq_receive");
                exit(1);
            }
            buf[n] = '\0';

            if (strcmp(buf, "quit") == 0) {
                printf("[부모] 자식이 채팅 종료\n");
                break;
            }

            printf("[부모 수신] %s\n\n", buf);
        }

        wait(NULL);
        mq_close(q_p2c);
        mq_close(q_c2p);
        mq_unlink(Q_P2C); // 커널에서 메시지 큐 삭제
        mq_unlink(Q_C2P);
        printf("[부모] 종료\n");
    }

    return 0;
}