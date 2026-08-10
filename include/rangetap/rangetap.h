#ifndef RNTP_RANGETAP_H
#define RNTP_RANGETAP_H

#include <stdint.h>
#include <string.h>

#define RNTP_BACKEND_NONE 0
#define RNTP_BACKEND_NVTX 1
#define RNTP_BACKEND_ITT 2

#if defined(RNTP_ENABLE_NVTX) && defined(RNTP_ENABLE_ITT)
#error "Define only one of RNTP_ENABLE_NVTX or RNTP_ENABLE_ITT"
#endif

#if !defined(RNTP_BACKEND)
#if defined(RNTP_ENABLE_NVTX)
#define RNTP_BACKEND RNTP_BACKEND_NVTX
#elif defined(RNTP_ENABLE_ITT)
#define RNTP_BACKEND RNTP_BACKEND_ITT
#endif
#endif

#ifndef RNTP_BACKEND
#define RNTP_BACKEND RNTP_BACKEND_NONE
#endif

#if RNTP_BACKEND == RNTP_BACKEND_NVTX
#include <nvtx3/nvToolsExt.h>
#elif RNTP_BACKEND == RNTP_BACKEND_ITT
#include <ittnotify.h>
#endif

#if RNTP_BACKEND != RNTP_BACKEND_NONE && \
    RNTP_BACKEND != RNTP_BACKEND_NVTX && \
    RNTP_BACKEND != RNTP_BACKEND_ITT
#error "RNTP_BACKEND must be RNTP_BACKEND_NONE, RNTP_BACKEND_NVTX, or RNTP_BACKEND_ITT"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t RNTP_Color;

#if RNTP_BACKEND == RNTP_BACKEND_NVTX
typedef struct RNTP_RangeHandle {
    nvtxRangeId_t value;
} RNTP_RangeHandle;
#elif RNTP_BACKEND == RNTP_BACKEND_ITT
typedef struct RNTP_RangeHandle {
    __itt_id value;
} RNTP_RangeHandle;
#else
typedef struct RNTP_RangeHandle {
    uint64_t value;
} RNTP_RangeHandle;
#endif

static inline RNTP_Color RNTP_ColorARGB(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue) {
    return ((uint32_t)alpha << 24) |
           ((uint32_t)red << 16) |
           ((uint32_t)green << 8) |
           (uint32_t)blue;
}

#define RNTP_COLOR_ARGB(alpha, red, green, blue) RNTP_ColorARGB((alpha), (red), (green), (blue))
#define RNTP_COLOR_RGB(red, green, blue) RNTP_ColorARGB(0xFFu, (red), (green), (blue))

static inline RNTP_RangeHandle RNTP_RangeHandleInvalid(void) {
    RNTP_RangeHandle handle;
#if RNTP_BACKEND == RNTP_BACKEND_ITT
    handle.value = __itt_null;
#else
    handle.value = 0;
#endif
    return handle;
}

#if RNTP_BACKEND == RNTP_BACKEND_NVTX

static inline nvtxEventAttributes_t RNTP__NvtxEventAttributes(const char *name, RNTP_Color color) {
    nvtxEventAttributes_t event_attrib;

    memset(&event_attrib, 0, sizeof(event_attrib));
    event_attrib.version = NVTX_VERSION;
    event_attrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
    event_attrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
    event_attrib.message.ascii = name;

    if (color != 0) {
        event_attrib.colorType = NVTX_COLOR_ARGB;
        event_attrib.color = color;
    }

    return event_attrib;
}

static inline void RNTP_PushMark(const char *name) {
    nvtxRangePushA(name);
}

static inline void RNTP_PushMarkEx(const char *name, RNTP_Color color) {
    nvtxEventAttributes_t event_attrib = RNTP__NvtxEventAttributes(name, color);
    nvtxRangePushEx(&event_attrib);
}

static inline void RNTP_PopMark(void) {
    (void)nvtxRangePop();
}

static inline RNTP_RangeHandle RNTP_RangeBegin(const char *name) {
    RNTP_RangeHandle handle;
    nvtxEventAttributes_t event_attrib = RNTP__NvtxEventAttributes(name, 0);

    handle.value = nvtxRangeStartEx(&event_attrib);
    return handle;
}

static inline RNTP_RangeHandle RNTP_RangeBeginEx(const char *name, RNTP_Color color) {
    RNTP_RangeHandle handle;
    nvtxEventAttributes_t event_attrib = RNTP__NvtxEventAttributes(name, color);

    handle.value = nvtxRangeStartEx(&event_attrib);
    return handle;
}

static inline void RNTP_RangeEnd(RNTP_RangeHandle handle) {
    nvtxRangeEnd(handle.value);
}

#elif RNTP_BACKEND == RNTP_BACKEND_ITT

static inline __itt_domain *RNTP__IttDomain(void) {
    static __itt_domain *domain = 0;

    if (domain == 0) {
        domain = __itt_domain_create("RangeTap");
    }

    return domain;
}

static inline __itt_string_handle *RNTP__IttStringHandle(const char *name) {
    return __itt_string_handle_create(name);
}

static inline void *RNTP__IttAnchor(void) {
    static int anchor = 0;
    return &anchor;
}

static inline unsigned long long RNTP__IttNextIdValue(void) {
    static unsigned long long counter = 1;

#if defined(__GNUC__) || defined(__clang__)
    return __atomic_fetch_add(&counter, 1ULL, __ATOMIC_RELAXED);
#else
    unsigned long long next_value = counter;
    counter += 1;
    return next_value;
#endif
}

static inline __itt_id RNTP__IttMakeId(void) {
    return __itt_id_make(RNTP__IttAnchor(), RNTP__IttNextIdValue());
}

static inline void RNTP_PushMark(const char *name) {
    __itt_domain *domain = RNTP__IttDomain();
    __itt_string_handle *string_handle = RNTP__IttStringHandle(name);

    __itt_task_begin(domain, __itt_null, __itt_null, string_handle);
}

static inline void RNTP_PushMarkEx(const char *name, RNTP_Color color) {
    (void)color;
    RNTP_PushMark(name);
}

static inline void RNTP_PopMark(void) {
    __itt_task_end(RNTP__IttDomain());
}

static inline RNTP_RangeHandle RNTP_RangeBegin(const char *name) {
    RNTP_RangeHandle handle;
    __itt_domain *domain = RNTP__IttDomain();
    __itt_string_handle *string_handle = RNTP__IttStringHandle(name);

    handle.value = RNTP__IttMakeId();
    __itt_id_create(domain, handle.value);
    __itt_task_begin_overlapped(domain, handle.value, __itt_null, string_handle);
    return handle;
}

static inline RNTP_RangeHandle RNTP_RangeBeginEx(const char *name, RNTP_Color color) {
    (void)color;
    return RNTP_RangeBegin(name);
}

static inline void RNTP_RangeEnd(RNTP_RangeHandle handle) {
    __itt_domain *domain = RNTP__IttDomain();

    __itt_task_end_overlapped(domain, handle.value);
    __itt_id_destroy(domain, handle.value);
}

#else

static inline void RNTP_PushMark(const char *name) {
    (void)name;
}

static inline void RNTP_PushMarkEx(const char *name, RNTP_Color color) {
    (void)name;
    (void)color;
}

static inline void RNTP_PopMark(void) {
}

static inline RNTP_RangeHandle RNTP_RangeBegin(const char *name) {
    (void)name;
    return RNTP_RangeHandleInvalid();
}

static inline RNTP_RangeHandle RNTP_RangeBeginEx(const char *name, RNTP_Color color) {
    (void)name;
    (void)color;
    return RNTP_RangeHandleInvalid();
}

static inline void RNTP_RangeEnd(RNTP_RangeHandle handle) {
    (void)handle;
}

#endif

#define RNTP_PUSH(name) RNTP_PushMark(name)
#define RNTP_PUSH_COLOR(name, color) RNTP_PushMarkEx((name), (color))
#define RNTP_POP() RNTP_PopMark()
#define RNTP_RANGE_BEGIN(name) RNTP_RangeBegin(name)
#define RNTP_RANGE_BEGIN_COLOR(name, color) RNTP_RangeBeginEx((name), (color))
#define RNTP_RANGE_END(handle) RNTP_RangeEnd(handle)

#ifdef __cplusplus
}
#endif

#endif
