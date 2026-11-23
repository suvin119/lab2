#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

#define SHMNAME "/my_copy_shm"
#define BUFSIZE 1024

// 공유 메모리에 올라갈 데이터 구조
struct shm_data {
    sem_t sem_write; // 부모 쓰기 허용 세마포어
    sem_t sem_read; // 자식 읽기 허용 세마포어
    ssize_t len; // 읽은 바이트 수 (0이면 EOF)
    char buf[BUFSIZE]; // 파일 데이터 버퍼
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        exit(1);
    }

    // 이전 실행에서 남은 공유 메모리가 있다면 미리 제거
    shm_unlink(SHMNAME);

    // POSIX 공유 메모리 객체 생성
    int shmd = shm_open(SHMNAME, O_CREAT | O_RDWR, 0666);
    if (shmd == -1) {
        perror("shm_open");
        exit(1);
    }

    // 공유 메모리 크기 설정
    if (ftruncate(shmd, sizeof(struct shm_data)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // 프로세스 주소 공간에 mmap으로 매핑
    struct shm_data *shm = (struct shm_data *)mmap(
        NULL, sizeof(struct shm_data),
        PROT_READ | PROT_WRITE, MAP_SHARED, shmd, 0
    );
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // 프로세스 간 공유 세마포어 초기화 (pshared = 1)
    sem_init(&shm->sem_write, 1, 1); // 처음엔 쓰기 가능(1)
    sem_init(&shm->sem_read, 1, 0);  // 처음엔 읽기 대기(0)

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // 자식: 공유 메모리 -> 대상 파일(argv[2]) 쓰기
        int dst_fd = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (dst_fd == -1) {
            perror("open dest");
            exit(1);
        }

        while (1) {
            sem_wait(&shm->sem_read); // 부모가 공유 메모리에 쓸 때까지 대기

            if (shm->len <= 0) { // 파일 끝(EOF) 도달 시 종료
                sem_post(&shm->sem_write);
                break;
            }

            if (write(dst_fd, shm->buf, shm->len) != shm->len) {
                perror("write dest");
                exit(1);
            }

            sem_post(&shm->sem_write); // 부모에게 다음 데이터 쓰기 허용
        }

        close(dst_fd);
        munmap(shm, sizeof(struct shm_data));
        close(shmd);
        exit(0);
    } else {
        // 부모: 원본 파일(argv[1]) 읽기 -> 공유 메모리 쓰기
        int src_fd = open(argv[1], O_RDONLY);
        if (src_fd == -1) {
            perror("open src");
            shm_unlink(SHMNAME);
            exit(1);
        }

        while (1) {
            sem_wait(&shm->sem_write); // 자식이 이전 데이터를 읽을 때까지 대기

            shm->len = read(src_fd, shm->buf, BUFSIZE);
            if (shm->len < 0) {
                perror("read src");
                exit(1);
            }

            sem_post(&shm->sem_read); // 자식에게 읽기 허용

            if (shm->len == 0) { // EOF 전달 후 반복 종료
                break;
            }
        }

        wait(NULL);
        close(src_fd);

        // 자원 정리
        sem_destroy(&shm->sem_write);
        sem_destroy(&shm->sem_read);
        munmap(shm, sizeof(struct shm_data));
        close(shmd);
        shm_unlink(SHMNAME);

        printf("파일 복사 완료: %s -> %s\n", argv[1], argv[2]);
    }

    return 0;
}