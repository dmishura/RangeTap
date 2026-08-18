#include <rangetap/rangetap.h>

#include <stdint.h>
#include <time.h>

static volatile uint64_t result_sink;

static void sleep_milliseconds(long milliseconds) {
    struct timespec delay;

    delay.tv_sec = milliseconds / 1000;
    delay.tv_nsec = (milliseconds % 1000) * 1000000L;
    nanosleep(&delay, 0);
}

static void cpu_work(uint64_t iterations) {
    uint64_t value = result_sink + 1;
    uint64_t index;

    for (index = 0; index < iterations; ++index) {
        value = value * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
        value ^= value >> 17;
    }

    result_sink = value;
}

int main(void) {
    RNTP_RangeHandle whole_run = rntp_range_start_ex(
        "RangeTap runtime smoke test",
        RNTP_COLOR_RGB(0x33, 0x99, 0xFF));

    RNTP_PushMark("warmup");
    cpu_work(UINT64_C(10000000));
    RNTP_PopMark();

    RNTP_PushMarkEx("nested work", RNTP_COLOR_RGB(0xFF, 0x99, 0x22));
    sleep_milliseconds(150);

    RNTP_PushMarkEx("CPU work", RNTP_COLOR_RGB(0x44, 0xCC, 0x66));
    cpu_work(UINT64_C(50000000));
    RNTP_PopMark();

    RNTP_PushMark("observable sleep");
    sleep_milliseconds(250);
    RNTP_PopMark();
    RNTP_PopMark();

    rntp_range_end(&whole_run);
    rntp_range_end(&whole_run);
    return result_sink == 0 || rntp_range_is_open(whole_run);
}
