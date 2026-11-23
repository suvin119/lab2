#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    int fd;
    char *msg = "Hello File I/O!\n";
    char buf[128] = {0};

    // 파일 생성 및 열기 (권한 0644)
    fd = open("test.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    // 파일에 쓰기
    write(fd, msg, strlen(msg));

    // 읽기 위해 파일 포인터를 맨 앞으로 이동 (lseek)
    lseek(fd, 0, SEEK_SET);

    // 파일에서 읽기 및 출력
    read(fd, buf, sizeof(buf) - 1);
    printf("읽은 내용: %s", buf);

    close(fd);
    return 0;
}