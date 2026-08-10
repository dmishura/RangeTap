#include <rangetap/rangetap.h>

static void nested_region(void) {
    RNTP_RangeHandle range = RNTP_RangeBegin("none-range");

    RNTP_PushMark("none-outer");
    RNTP_PushMarkEx("none-inner", RNTP_COLOR_RGB(0x33, 0x66, 0x99));
    RNTP_PopMark();
    RNTP_PopMark();
    RNTP_RangeEnd(range);
}

int main(void) {
    nested_region();
    return 0;
}
