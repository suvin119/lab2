#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-bus.h>

static uint32_t notify_count = 0;

static int send_notify(sd_bus_message *m, void *userdata, sd_bus_error *error) {
    const char *msg = NULL;
    char reply[256];

    // 인자 1개(string) 읽기
    int ret = sd_bus_message_read(m, "s", &msg);
    if (ret < 0) {
        fprintf(stderr, "인자 파싱 실패: %s\n", strerror(-ret));
        return ret;
    }

    notify_count++;
    printf("[알림 #%u] %s\n", notify_count, msg);

    snprintf(reply, sizeof(reply), "알림 수신 완료 (총 %u건)", notify_count);
    return sd_bus_reply_method_return(m, "s", reply);
}

// D-Bus 인터페이스 vtable 정의
static const sd_bus_vtable notify_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD_WITH_ARGS(
        "SendNotify",
        SD_BUS_ARGS("s", msg),
        SD_BUS_RESULT("s", reply),
        send_notify,
        SD_BUS_VTABLE_UNPRIVILEGED
    ),
    SD_BUS_VTABLE_END
};

int main(void) {
    sd_bus_slot *slot = NULL;
    sd_bus *bus = NULL;
    int ret;

    // 사용자 세션 버스 연결
    ret = sd_bus_default_user(&bus);
    if (ret < 0) {
        fprintf(stderr, "세션 버스 연결 실패: %s\n", strerror(-ret));
        goto finish;
    }

    // 객체 경로와 인터페이스 등록
    ret = sd_bus_add_object_vtable(
        bus, &slot,
        "/org/example/NotifyService",
        "org.example.NotifyService",
        notify_vtable, NULL
    );
    if (ret < 0) {
        fprintf(stderr, "인터페이스 등록 실패: %s\n", strerror(-ret));
        goto finish;
    }

    // D-Bus 서비스 이름 요청
    ret = sd_bus_request_name(bus, "org.example.NotifyService", 0);
    if (ret < 0) {
        fprintf(stderr, "서비스 이름 획득 실패: %s\n", strerror(-ret));
        goto finish;
    }

    // 메시지 루프
    for (;;) {
        ret = sd_bus_process(bus, NULL);
        if (ret < 0) {
            fprintf(stderr, "버스 처리 오류: %s\n", strerror(-ret));
            break;
        }
        if (ret == 0) {
            sd_bus_wait(bus, (uint64_t)(1000 * 1000));
        }
    }

finish:
    sd_bus_slot_unref(slot);
    sd_bus_unref(bus);
    return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}