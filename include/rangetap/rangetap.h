#ifndef RNTP_RANGETAP_H
#define RNTP_RANGETAP_H

#include <stdint.h>
#include <string.h>

#define RNTP_BACKEND_NONE 0
#define RNTP_BACKEND_NVTX 1
#define RNTP_BACKEND_ITT 2
#define RNTP_BACKEND_STREAMLINE 3

#if (defined(RNTP_ENABLE_NVTX) + defined(RNTP_ENABLE_ITT) + defined(RNTP_ENABLE_STREAMLINE)) > 1
#error "Define only one RNTP_ENABLE_* backend flag"
#endif

#if !defined(RNTP_BACKEND)
#if defined(RNTP_ENABLE_NVTX)
#define RNTP_BACKEND RNTP_BACKEND_NVTX
#elif defined(RNTP_ENABLE_ITT)
#define RNTP_BACKEND RNTP_BACKEND_ITT
#elif defined(RNTP_ENABLE_STREAMLINE)
#define RNTP_BACKEND RNTP_BACKEND_STREAMLINE
#endif
#endif

#ifndef RNTP_BACKEND
#define RNTP_BACKEND RNTP_BACKEND_NONE
#endif

#if RNTP_BACKEND == RNTP_BACKEND_NVTX
#include <nvtx3/nvToolsExt.h>
#elif RNTP_BACKEND == RNTP_BACKEND_ITT
#include <ittnotify.h>
#elif RNTP_BACKEND == RNTP_BACKEND_STREAMLINE
#include <streamline_annotate.h>
#endif

#if RNTP_BACKEND != RNTP_BACKEND_NONE && \
    RNTP_BACKEND != RNTP_BACKEND_NVTX && \
    RNTP_BACKEND != RNTP_BACKEND_ITT && \
    RNTP_BACKEND != RNTP_BACKEND_STREAMLINE
#error "RNTP_BACKEND must name a supported RangeTap backend"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if RNTP_BACKEND == RNTP_BACKEND_NVTX
typedef struct RNTP_RangeHandle {
    nvtxRangeId_t value;
} RNTP_RangeHandle;
#elif RNTP_BACKEND == RNTP_BACKEND_ITT
typedef struct RNTP_RangeHandle {
    __itt_id value;
} RNTP_RangeHandle;
#elif RNTP_BACKEND == RNTP_BACKEND_STREAMLINE
typedef struct RNTP_RangeHandle {
    uint32_t value;
} RNTP_RangeHandle;
#else
typedef struct RNTP_RangeHandle {
    uint64_t value;
} RNTP_RangeHandle;
#endif

static inline uint32_t RNTP_ColorARGB(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue) {
    return ((uint32_t)alpha << 24) |
           ((uint32_t)red << 16) |
           ((uint32_t)green << 8) |
           (uint32_t)blue;
}

#define RNTP_COLOR_ARGB(alpha, red, green, blue) RNTP_ColorARGB((alpha), (red), (green), (blue))
#define RNTP_COLOR_RGB(red, green, blue) RNTP_ColorARGB(0xFFu, (red), (green), (blue))
#define RNTP_COLOR_RED UINT32_C(0xFFFF0000)
#define RNTP_COLOR_GREEN UINT32_C(0xFF00FF00)
#define RNTP_COLOR_BLUE UINT32_C(0xFF0000FF)
#define RNTP_COLOR_YELLOW UINT32_C(0xFFFFFF00)
#define RNTP_COLOR_CYAN UINT32_C(0xFF00FFFF)
#define RNTP_COLOR_MAGENTA UINT32_C(0xFFFF00FF)
#define RNTP_COLOR_ORANGE UINT32_C(0xFFFF8000)
#define RNTP_COLOR_PURPLE UINT32_C(0xFF8000FF)

static inline RNTP_RangeHandle RNTP__RangeHandleClosed(void) {
    RNTP_RangeHandle handle;
#if RNTP_BACKEND == RNTP_BACKEND_ITT
    handle.value = __itt_null;
#else
    handle.value = 0;
#endif
    return handle;
}

static inline int rntp_range_is_open(RNTP_RangeHandle handle) {
#if RNTP_BACKEND == RNTP_BACKEND_ITT
    return handle.value.d1 != 0 || handle.value.d2 != 0 || handle.value.d3 != 0;
#else
    return handle.value != 0;
#endif
}

#if RNTP_BACKEND == RNTP_BACKEND_NVTX

static inline nvtxEventAttributes_t RNTP__NvtxEventAttributes(const char *name, uint32_t color) {
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

static inline void RNTP_PushMarkEx(const char *name, uint32_t color) {
    nvtxEventAttributes_t event_attrib = RNTP__NvtxEventAttributes(name, color);
    nvtxRangePushEx(&event_attrib);
}

static inline void RNTP_PopMark(void) {
    (void)nvtxRangePop();
}

static inline RNTP_RangeHandle rntp_range_start(const char *name) {
    RNTP_RangeHandle handle;
    nvtxEventAttributes_t event_attrib = RNTP__NvtxEventAttributes(name, 0);

    handle.value = nvtxRangeStartEx(&event_attrib);
    return handle;
}

static inline RNTP_RangeHandle rntp_range_start_ex(const char *name, uint32_t color) {
    RNTP_RangeHandle handle;
    nvtxEventAttributes_t event_attrib = RNTP__NvtxEventAttributes(name, color);

    handle.value = nvtxRangeStartEx(&event_attrib);
    return handle;
}

static inline void rntp_range_end(RNTP_RangeHandle *handle) {
    if (handle == 0 || !rntp_range_is_open(*handle)) {
        return;
    }

    nvtxRangeEnd(handle->value);
    *handle = RNTP__RangeHandleClosed();
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

static inline void RNTP_PushMarkEx(const char *name, uint32_t color) {
    (void)color;
    RNTP_PushMark(name);
}

static inline void RNTP_PopMark(void) {
    __itt_task_end(RNTP__IttDomain());
}

static inline RNTP_RangeHandle rntp_range_start(const char *name) {
    RNTP_RangeHandle handle;
    __itt_domain *domain = RNTP__IttDomain();
    __itt_string_handle *string_handle = RNTP__IttStringHandle(name);

    handle.value = RNTP__IttMakeId();
    __itt_id_create(domain, handle.value);
    __itt_task_begin_overlapped(domain, handle.value, __itt_null, string_handle);
    return handle;
}

static inline RNTP_RangeHandle rntp_range_start_ex(const char *name, uint32_t color) {
    (void)color;
    return rntp_range_start(name);
}

static inline void rntp_range_end(RNTP_RangeHandle *handle) {
    __itt_domain *domain;

    if (handle == 0 || !rntp_range_is_open(*handle)) {
        return;
    }

    domain = RNTP__IttDomain();
    __itt_task_end_overlapped(domain, handle->value);
    __itt_id_destroy(domain, handle->value);
    *handle = RNTP__RangeHandleClosed();
}

#elif RNTP_BACKEND == RNTP_BACKEND_STREAMLINE

#if defined(_MSC_VER)
#define RNTP__THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
#define RNTP__THREAD_LOCAL __thread
#else
#define RNTP__THREAD_LOCAL _Thread_local
#endif

static RNTP__THREAD_LOCAL uint32_t RNTP__StreamlinePushDepth;
static RNTP__THREAD_LOCAL uint32_t RNTP__StreamlineNextRange = UINT32_C(0x80000000);

static inline uint32_t RNTP__StreamlineColor(uint32_t color) {
    return UINT32_C(0x1B) |
           ((color >> 8) & UINT32_C(0x0000FF00)) |
           ((color << 8) & UINT32_C(0x00FF0000)) |
           ((color << 24) & UINT32_C(0xFF000000));
}

static inline void RNTP__StreamlineBegin(uint32_t channel, const char *name, uint32_t color) {
    gator_annotate_setup();
    if (color == 0) {
        gator_annotate_str(channel, name);
    } else {
        gator_annotate_color(channel, RNTP__StreamlineColor(color), name);
    }
}

static inline void RNTP_PushMark(const char *name) {
    RNTP__StreamlinePushDepth += 1;
    RNTP__StreamlineBegin(RNTP__StreamlinePushDepth, name, 0);
}

static inline void RNTP_PushMarkEx(const char *name, uint32_t color) {
    RNTP__StreamlinePushDepth += 1;
    RNTP__StreamlineBegin(RNTP__StreamlinePushDepth, name, color);
}

static inline void RNTP_PopMark(void) {
    if (RNTP__StreamlinePushDepth != 0) {
        gator_annotate_str(RNTP__StreamlinePushDepth, 0);
        RNTP__StreamlinePushDepth -= 1;
    }
}

static inline RNTP_RangeHandle rntp_range_start(const char *name) {
    RNTP_RangeHandle handle;
    handle.value = RNTP__StreamlineNextRange++;
    RNTP__StreamlineBegin(handle.value, name, 0);
    return handle;
}

static inline RNTP_RangeHandle rntp_range_start_ex(const char *name, uint32_t color) {
    RNTP_RangeHandle handle;
    handle.value = RNTP__StreamlineNextRange++;
    RNTP__StreamlineBegin(handle.value, name, color);
    return handle;
}

static inline void rntp_range_end(RNTP_RangeHandle *handle) {
    if (handle == 0 || !rntp_range_is_open(*handle)) {
        return;
    }

    gator_annotate_str(handle->value, 0);
    *handle = RNTP__RangeHandleClosed();
}

#else

static inline void RNTP_PushMark(const char *name) {
    (void)name;
}

static inline void RNTP_PushMarkEx(const char *name, uint32_t color) {
    (void)name;
    (void)color;
}

static inline void RNTP_PopMark(void) {
}

static inline RNTP_RangeHandle rntp_range_start(const char *name) {
    (void)name;
    return RNTP__RangeHandleClosed();
}

static inline RNTP_RangeHandle rntp_range_start_ex(const char *name, uint32_t color) {
    (void)name;
    (void)color;
    return RNTP__RangeHandleClosed();
}

static inline void rntp_range_end(RNTP_RangeHandle *handle) {
    if (handle != 0) {
        *handle = RNTP__RangeHandleClosed();
    }
}

#endif

#define RNTP_PUSH(name) RNTP_PushMark(name)
#define RNTP_PUSH_COLOR(name, color) RNTP_PushMarkEx((name), (color))
#define RNTP_POP() RNTP_PopMark()
#define RNTP_RANGE_START(name) rntp_range_start(name)
#define RNTP_RANGE_START_COLOR(name, color) rntp_range_start_ex((name), (color))
#define RNTP_RANGE_IS_OPEN(handle) rntp_range_is_open(handle)
#define RNTP_RANGE_END(handle) rntp_range_end(&(handle))

#ifdef __cplusplus
}
#endif

#endif
