#include <rangetap/rangetap.h>

static void nested_region(void) {
    RNTP_RangeHandle range = rntp_range_start("itt-range");

    RNTP_PushMark("itt-outer");
    RNTP_PushMarkEx("itt-inner", RNTP_COLOR_RGB(0xFF, 0xCC, 0x00));
    RNTP_PopMark();
    RNTP_PopMark();
    rntp_range_end(&range);
    rntp_range_end(&range);
}

int main(void) {
    nested_region();
    return 0;
}
