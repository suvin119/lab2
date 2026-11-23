#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SENTENCES 3
#define MAX_LENGTH 256  

const char *sentences[MAX_SENTENCES] = {
    "동해물과 백두산이 마르고 닳도록",
    "하느님이 보우하사 우리나라 만세",
    "무궁화 삼천리 화려강산"
};

void typing_practice();
int calculate_errors(const char *correct, const char *input);

int main() {
    printf("다음 문장을 그대로 입력하세요.\n");
    typing_practice();
    return 0;
}

void typing_practice() {
    char user_input[MAX_LENGTH];
    int total_errors = 0;
    double total_time = 0;

    for (int i = 0; i < MAX_SENTENCES; i++) {
        printf("문장 : %s\n", sentences[i]);

        // 입력 시작 시간 기록 (time()을 사용하여 초 단위 시간 측정)
        time_t start_time = time(NULL);

        // 사용자 입력 받기
        fgets(user_input, sizeof(user_input), stdin);
        user_input[strcspn(user_input, "\n")] = '\0';

        // 입력 종료 시간 기록
        time_t end_time = time(NULL);

        // 타이핑 시간 계산 (초 단위)
        double elapsed_time = difftime(end_time, start_time);
        total_time += (elapsed_time > 0.01 ? elapsed_time : 0.01); 

        // 오류 횟수 계산
        int errors = calculate_errors(sentences[i], user_input);
        total_errors += errors;

        printf("오류 수: %d, 소요 시간: %.2f초\n\n", errors, elapsed_time);
    }

    // WPM 계산
    double total_words = 0;
    for (int i = 0; i < MAX_SENTENCES; i++) {
        total_words += strlen(sentences[i]) / 5.0;
    }
    double wpm = (total_words / total_time) * 60;

    printf("총 오류 수: %d\n", total_errors);
    printf("평균 분당 타자수(WPM): %.2f\n", wpm);
}

// 오류 계산 함수
int calculate_errors(const char *correct, const char *input) {
    size_t len_correct = strlen(correct);
    size_t len_input = strlen(input);
    int errors = 0;

    // 최대 길이로 비교
    size_t max_len = len_correct > len_input ? len_correct : len_input;
    for (size_t i = 0; i < max_len; i++) {
        if (i >= len_input || i >= len_correct || correct[i] != input[i]) {
            errors++;
        }
    }
    return errors;
}