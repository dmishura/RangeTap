# RangeTap

RangeTap is a header-only C library that provides a small unified API for profiling range annotations across multiple backends.

The first supported backends are:
- NVIDIA NVTX for Nsight-based workflows;
- Intel ITT for VTune-based workflows;
- a no-op fallback backend for builds where profiling annotations are disabled.

## Status

This is an early API skeleton focused on the smallest common feature set:
- nested push/pop regions;
- explicit begin/end ranges;
- optional color annotations;
- compile-time backend selection.

## Public API

Include:

```c
#include <rangetap/rangetap.h>
```

Core functions:

```c
void RNTP_PushMark(const char *name);
void RNTP_PushMarkEx(const char *name, RNTP_Color color);
void RNTP_PopMark(void);

RNTP_RangeHandle RNTP_RangeBegin(const char *name);
RNTP_RangeHandle RNTP_RangeBeginEx(const char *name, RNTP_Color color);
void RNTP_RangeEnd(RNTP_RangeHandle handle);
```

Convenience macros:

```c
RNTP_PUSH("outer");
RNTP_PUSH_COLOR("inner", RNTP_COLOR_RGB(0xFF, 0x80, 0x00));
RNTP_POP();

RNTP_RangeHandle range = RNTP_RANGE_BEGIN("step");
RNTP_RANGE_END(range);
```

## Backend Selection

RangeTap supports two ways to choose the backend at compile time.

Preferred backend flags:

```c
-DRNTP_ENABLE_NVTX
-DRNTP_ENABLE_ITT
```

Low-level backend selector:

```c
-DRNTP_BACKEND=RNTP_BACKEND_NVTX
-DRNTP_BACKEND=RNTP_BACKEND_ITT
-DRNTP_BACKEND=RNTP_BACKEND_NONE
```

If nothing is defined, RangeTap defaults to the no-op backend.

Do not define both `RNTP_ENABLE_NVTX` and `RNTP_ENABLE_ITT`.

## Backend Include Paths

RangeTap is header-only, but backend headers still need to be visible to the consumer build. The test build discovers standard Nsight and VTune installations under `/opt` automatically. Custom locations can be supplied with the `RNTP_NVTX_INCLUDE_DIR`, `RNTP_ITT_INCLUDE_DIR`, and `RNTP_ITT_LIBRARY` CMake cache variables.

### NVTX

Typical Nsight Systems include path:

```text
/opt/nvidia/nsight-systems-cli/<version>/target-linux-x64/nvtx/include
```

Example compile flags:

```sh
gcc -Iinclude \
    -I/opt/nvidia/nsight-systems-cli/<version>/target-linux-x64/nvtx/include \
    -DRNTP_ENABLE_NVTX \
    your_file.c
```

### ITT

Typical VTune SDK paths:

```text
/opt/intel/oneapi/vtune/<version>/sdk/include
/opt/intel/oneapi/vtune/<version>/sdk/lib64/libittnotify.a
```

Example compile flags:

```sh
gcc -Iinclude \
    -I/opt/intel/oneapi/vtune/<version>/sdk/include \
    -DRNTP_ENABLE_ITT \
    your_file.c \
    /opt/intel/oneapi/vtune/<version>/sdk/lib64/libittnotify.a \
    -ldl
```

## Example

Minimal example:

```c
#include <rangetap/rangetap.h>

int main(void) {
    RNTP_RangeHandle startup = RNTP_RangeBegin("startup");

    RNTP_PushMark("initialization");
    RNTP_PopMark();

    RNTP_RangeEnd(startup);
    return 0;
}
```

See [examples/hello_world.c](/home/dmishura/RangeTap/examples/hello_world.c).

## Runtime profiling smoke test

The `rntp_runtime_nvtx` and `rntp_runtime_itt` targets execute nested annotated regions containing CPU work and sleeps. The complete range lasts approximately 500 milliseconds, so every annotation is visible in a timeline.

Build and collect profiles with:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build

scripts/profile_nvtx.sh build rangetap-nvtx
scripts/profile_vtune.sh build rangetap-vtune
```

The NVTX script produces an `.nsys-rep` file. The VTune script produces a VTune result directory. Both scripts accept the build path as their first argument and the result path as their second argument.

## Current Semantics

- `name` is the only required portable parameter.
- `color` is best-effort. It is mapped directly for NVTX and currently ignored for ITT.
- `tid` is intentionally not part of the v1 public API yet because it does not map cleanly across both backends.

## Repository Notes

- The project policy is English-only documentation and code comments.
- Profiler SDKs are external dependencies and are not part of the RangeTap source tree.
