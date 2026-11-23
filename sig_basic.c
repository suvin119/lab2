#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sig_handler(int signo) {
    if (signo == SIGINT) {
        printf("\n[시그널] 감지\n");
    } else if (signo == SIGALRM) {
        printf("\n[시그널] 3초 알람(SIGALRM)\n");
    }
}

int main(void) {
    struct sigaction act;

    act.sa_handler = sig_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGINT, &act, NULL);  
    sigaction(SIGALRM, &act, NULL); 

    // 3초 뒤 SIGALRM 발생 예약 후 대기
    alarm(3);
    pause(); // 시그널이 올 때까지 대기

    printf("프로그램 정상 종료\n");
    return 0;
}