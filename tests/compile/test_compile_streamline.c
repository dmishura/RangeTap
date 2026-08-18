#include <rangetap/rangetap.h>

static void nested_region(void) {
    RNTP_RangeHandle range = rntp_range_start_ex(
        "streamline-range", RNTP_COLOR_RGB(0x00, 0xA8, 0xFF));

    RNTP_PushMark("streamline-outer");
    RNTP_PushMarkEx("streamline-inner", RNTP_COLOR_RGB(0xFF, 0x66, 0x33));
    RNTP_PopMark();
    RNTP_PopMark();
    rntp_range_end(&range);
    rntp_range_end(&range);
}

int main(void) {
    nested_region();
    return 0;
}
