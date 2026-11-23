#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SHMSIZE 1024

int main(void) {
    // 공유 메모리 생성 (IPC_PRIVATE: 부모-자식 간 전용 키)
    int shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    pid_t pid = fork();

    if (pid == 0) {
        // 자식 프로세스: 공유 메모리 부착 후 데이터 기록
        char *shmaddr = (char *)shmat(shmid, NULL, 0);
        strcpy(shmaddr, "Hello from shared memory!");
        printf("[자식] 데이터 기록 완료\n");
        shmdt(shmaddr); // 분리
        exit(0);
    } else if (pid > 0) {
        // 부모 프로세스: 자식 종료 대기 후 공유 메모리 읽기 및 삭제
        wait(NULL);
        char *shmaddr = (char *)shmat(shmid, NULL, 0);
        printf("[부모] 공유 메모리에서 읽은 내용: %s\n", shmaddr);
        shmdt(shmaddr); // 분리
        shmctl(shmid, IPC_RMID, NULL); // 공유 메모리 제거
    }

    return 0;
}