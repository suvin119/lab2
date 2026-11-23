#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

int main(void) {
    char cwd[256];
    DIR *pdir;
    struct dirent *pde;

    // 새 디렉토리 생성 및 이동
    mkdir("test_dir", 0755);
    chdir("test_dir");

    // 현재 작업 디렉토리 경로 출력
    getcwd(cwd, sizeof(cwd));
    printf("현재 작업 디렉토리: %s\n", cwd);

    // 상위 디렉토리(..) 열어서 내부 항목 나열
    pdir = opendir("..");
    if (pdir == NULL) {
        perror("opendir");
        exit(1);
    }

    printf("\n상위 디렉토리 목록:\n");
    while ((pde = readdir(pdir)) != NULL) {
        printf("%s\n", pde->d_name);
    }

    closedir(pdir);
    return 0;
}