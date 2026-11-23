#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-bus.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }

    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    const char *reply = NULL;
    int ret;

    // 사용자 세션 버스 연결
    ret = sd_bus_default_user(&bus);
    if (ret < 0) {
        fprintf(stderr, "세션 버스 연결 실패: %s\n", strerror(-ret));
        goto finish;
    }

    // 서버의 SendNotify 원격 메소드 호출
    ret = sd_bus_call_method(
        bus,
        "org.example.NotifyService", // 버스 이름
        "/org/example/NotifyService", // 객체 경로
        "org.example.NotifyService", // 인터페이스 이름
        "SendNotify", // 메소드 이름
        &error,
        &m,
        "s", // 인자 형식: 문자열 1개
        argv[1] // 인자 값: 알림 메시지
    );
    if (ret < 0) {
        fprintf(stderr, "메소드 호출 실패: %s\n", error.message ? error.message : strerror(-ret));
        goto finish;
    }

    // 서버 반환값 읽기
    ret = sd_bus_message_read(m, "s", &reply);
    if (ret < 0) {
        fprintf(stderr, "응답 파싱 실패: %s\n", strerror(-ret));
        goto finish;
    }

    printf("[서버 응답] %s\n", reply);

finish:
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_unref(bus);
    return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}