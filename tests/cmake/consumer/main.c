#define RNTP_STRINGIFY_IMPL(value) #value
#define RNTP_STRINGIFY(value) RNTP_STRINGIFY_IMPL(value)
static const char selected_backend[] = RNTP_STRINGIFY(RNTP_BACKEND);
int main(void) { return selected_backend[0] == '\0'; }
