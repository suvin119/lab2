#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    struct stat st;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    // 파일 존재 및 읽기 권한 확인
    if (access(argv[1], F_OK | R_OK) == 0) {
        printf("[%s] 존재하며 읽기 가능합니다.\n", argv[1]);
    }

    // 파일 유형 및 크기 확인
    if (stat(argv[1], &st) == 0) {
        printf("파일 크기: %ld bytes\n", st.st_size);
        if (S_ISREG(st.st_mode)) printf("유형: 일반 파일\n");
        else if (S_ISDIR(st.st_mode)) printf("유형: 디렉토리\n");
    }

    return 0;
}