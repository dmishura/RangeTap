# RangeTap

RangeTap is a header-only C library that provides a small unified API for profiling range annotations across multiple backends.

The first supported backends are:
- NVIDIA NVTX for Nsight-based workflows;
- Intel ITT for VTune-based workflows;
- Arm Streamline annotations for Arm Performance Studio workflows;
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
void RNTP_PushMarkEx(const char *name, uint32_t color);
void RNTP_PopMark(void);

RNTP_RangeHandle rntp_range_start(const char *name);
RNTP_RangeHandle rntp_range_start_ex(const char *name, uint32_t color);
int rntp_range_is_open(RNTP_RangeHandle handle);
void rntp_range_end(RNTP_RangeHandle *handle);
```

Convenience macros:

```c
RNTP_PUSH("outer");
RNTP_PUSH_COLOR("inner", RNTP_COLOR_RGB(0xFF, 0x80, 0x00));
RNTP_POP();

RNTP_RangeHandle range = RNTP_RANGE_START("step");
RNTP_RANGE_END(range);

/* RNTP_RANGE_END consumes the handle and is safe to call again. */
RNTP_RANGE_END(range);
```

Eight predefined opaque colors are available in addition to `RNTP_COLOR_RGB` and
`RNTP_COLOR_ARGB`:

```c
RNTP_COLOR_RED
RNTP_COLOR_GREEN
RNTP_COLOR_BLUE
RNTP_COLOR_YELLOW
RNTP_COLOR_CYAN
RNTP_COLOR_MAGENTA
RNTP_COLOR_ORANGE
RNTP_COLOR_PURPLE
```

## CMake Integration

When RangeTap is part of the source tree, add it and select the backend for each
consumer target with `rangetap_link_backend`:

```cmake
add_subdirectory(external/RangeTap)

add_executable(my_app main.cpp)
rangetap_link_backend(my_app ITT)
```

The supported backend names are `NONE`, `NVTX`, `ITT`, and `STREAMLINE`. The
function finds only the selected backend SDK and supplies the matching compiler
definition, include directories, and link libraries. Do not set `RNTP_BACKEND`
separately when using this function.

RangeTap can also be obtained with `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
    RangeTap
    GIT_REPOSITORY https://github.com/dmishura/RangeTap.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(RangeTap)

add_executable(my_app main.cpp)
rangetap_link_backend(my_app NVTX)
```

Pin `GIT_TAG` to a release or commit for reproducible builds. RangeTap tests are
disabled automatically when the project is included with `add_subdirectory` or
`FetchContent`.

Custom SDK locations can be supplied as CMake cache variables:

```sh
cmake -S . -B build \
    -DRNTP_ITT_INCLUDE_DIR=/path/to/itt/include \
    -DRNTP_ITT_LIBRARY=/path/to/libittnotify.a
```

The corresponding variables are `RNTP_NVTX_INCLUDE_DIR`,
`RNTP_ITT_INCLUDE_DIR`, `RNTP_ITT_LIBRARY`, `RNTP_STREAMLINE_INCLUDE_DIR`, and
`RNTP_STREAMLINE_LIBRARY`.

## Manual Backend Selection

Projects that do not use the CMake helper can choose the backend directly with
compiler definitions.

Preferred backend flags:

```c
-DRNTP_ENABLE_NVTX
-DRNTP_ENABLE_ITT
-DRNTP_ENABLE_STREAMLINE
```

Low-level backend selector:

```c
-DRNTP_BACKEND=RNTP_BACKEND_NVTX
-DRNTP_BACKEND=RNTP_BACKEND_ITT
-DRNTP_BACKEND=RNTP_BACKEND_STREAMLINE
-DRNTP_BACKEND=RNTP_BACKEND_NONE
```

If nothing is defined, RangeTap defaults to the no-op backend.

Define at most one `RNTP_ENABLE_*` backend flag. Do not combine an
`RNTP_ENABLE_*` flag with `RNTP_BACKEND`.

## Manual Backend Include Paths

RangeTap is header-only, but backend headers still need to be visible to a
consumer that does not use `rangetap_link_backend`.

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

### Arm Streamline

Build the annotation library from the `annotate` directory in the Arm gator source tree, then compile and link with it:

```sh
gcc -Iinclude \
    -I$HOME/gator/annotate \
    -DRNTP_ENABLE_STREAMLINE \
    your_file.c \
    -L$HOME/gator/annotate -lstreamline_annotate -pthread
```

RangeTap initializes the annotation library on first use. Nested push/pop regions use separate Streamline channels. Explicit ranges must begin and end on the same thread because Streamline channels are thread-local. The repository only compile-links this backend on x86; runtime validation requires an Arm target with Streamline capture.

## Example

Minimal example:

```c
#include <rangetap/rangetap.h>

int main(void) {
    RNTP_RangeHandle startup = rntp_range_start("startup");

    RNTP_PushMark("initialization");
    RNTP_PopMark();

    rntp_range_end(&startup);
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
- Streamline receives RGB colors; the alpha component is ignored.
- `tid` is intentionally not part of the v1 public API yet because it does not map cleanly across both backends.
- `rntp_range_is_open` returns zero for a closed handle and a nonzero value for an open handle.
- `rntp_range_end` accepts a pointer, does nothing for a null or closed handle, and closes and invalidates an open handle.

## Repository Notes

- The project policy is English-only documentation and code comments.
- Profiler SDKs are external dependencies and are not part of the RangeTap source tree.
