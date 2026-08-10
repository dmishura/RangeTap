#include <rangetap/rangetap.h>

static void nested_region(void) {
    RNTP_RangeHandle range = RNTP_RangeBeginEx("nvtx-range", RNTP_COLOR_RGB(0x00, 0xA8, 0xFF));

    RNTP_PushMark("nvtx-outer");
    RNTP_PushMarkEx("nvtx-inner", RNTP_COLOR_RGB(0xFF, 0x66, 0x33));
    RNTP_PopMark();
    RNTP_PopMark();
    RNTP_RangeEnd(range);
}

int main(void) {
    nested_region();
    return 0;
}
