#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    pid_t pid;
    int status;

    pid = fork(); // 자식 프로세스 생성

    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // 자식 프로세스 영역
        printf("[자식] PID: %d, 부모 PID: %d\n", getpid(), getppid());
        execl("/bin/ls", "ls", "-l", (char *)0); // 새 프로그램으로 대체
        perror("execl"); // execl 실패 시에만
        exit(1);
    } else {
        // 부모 프로세스 영역
        printf("[부모] PID: %d, 생성한 자식 PID: %d -> 대기\n", getpid(), pid);
        wait(&status); // 자식이 끝날 때까지 대기
        if (WIFEXITED(status)) {
            printf("[부모] 자식 종료 확인 (종료 코드: %d)\n", WEXITSTATUS(status));
        }
    }

    return 0;
}