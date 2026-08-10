#include <rangetap/rangetap.h>

int main(void) {
    RNTP_RangeHandle startup = RNTP_RangeBegin("startup");

    RNTP_PushMark("initialization");
    RNTP_PopMark();

    RNTP_RangeEnd(startup);
    return 0;
}
