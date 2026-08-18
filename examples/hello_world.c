#include <rangetap/rangetap.h>

int main(void) {
    RNTP_RangeHandle startup = rntp_range_start("startup");

    RNTP_PushMark("initialization");
    RNTP_PopMark();

    rntp_range_end(&startup);
    return 0;
}
