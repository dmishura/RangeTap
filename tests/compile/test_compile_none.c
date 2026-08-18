#include <rangetap/rangetap.h>

static const uint32_t colors[] = {
    RNTP_COLOR_RED,
    RNTP_COLOR_GREEN,
    RNTP_COLOR_BLUE,
    RNTP_COLOR_YELLOW,
    RNTP_COLOR_CYAN,
    RNTP_COLOR_MAGENTA,
    RNTP_COLOR_ORANGE,
    RNTP_COLOR_PURPLE
};

static int nested_region(void) {
    RNTP_RangeHandle range = rntp_range_start("none-range");

    RNTP_PushMark("none-outer");
    RNTP_PushMarkEx("none-inner", colors[7]);
    RNTP_PopMark();
    RNTP_PopMark();
    rntp_range_end(&range);
    rntp_range_end(&range);
    rntp_range_end(0);
    return rntp_range_is_open(range);
}

int main(void) {
    return nested_region();
}
