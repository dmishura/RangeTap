# RangeTap Task

## Language Policy
All project documentation and all code comments must be written in English only.

## Goal
Create a library, in the broad sense a set of headers and object code, that provides a universal perf instrumentation API across multiple platforms.

The expected deliverables are:
- the library itself;
- examples for different platforms;
- tests, primarily compile-time checks, with possible cross-platform coverage later;
- concise documentation for the new API.

The library is needed to provide a single API for profiling region annotations. Each profiler currently exposes its own API: for example, VTune uses ITT, Nsight uses NVTX, and Streamline uses the Streamline Annotation API. This library will reduce those to one interface with implementation selection controlled by a compiler define.

## Context
The library is intended as an open-source project and will be published on GitHub.

Its initial use case is annotating profiling regions in my llama.cpp instrumentation project.

This task exists now because different profilers provide different capabilities, and there is a need to use them without rewriting the application code for each profiler-specific API.

The project starts from scratch.

Additional agreements:
- library name: RangeTap;
- public API prefix: `RNTP_`.

## Functional Requirements
The library must support profiling region annotation in two styles:
- push/pop style with nested regions;
- NVTX-like range style for explicit non-nested ranges.

The basic user scenario is annotating regions in an application to produce a profile with meaningful annotations.

The minimum viable version is the library plus test code that uses its API for region annotation. The basic success criterion is that a test using the RangeTap API compiles and links successfully.

ARM and Streamline support are explicitly out of scope for the first version because there is currently no hardware available for proper testing.

## Technical Inputs
The implementation language is plain C with a strong preference for being as lightweight as possible. The public API must be usable from both C and C++. Fortran support is postponed.

The build system will be CMake because the primary consumer project already uses CMake.

The first-version backends are VTune and Nsight.

Switching backends must not require rebuilding the library itself. Backend selection must happen when compiling the target application. The first implementation mechanism is a compiler define.

The target platforms for the first version are Linux x86_64 CPU and Linux x86_64 + CUDA.

## Data and Interface
The API must provide entities for two annotation modes:
- `PushMark` / `PopMark`;
- `RangeBegin` / `RangeEnd`.

The base call parameters are:
- `name`;
- `color`;
- `tid`.

The `name` parameter is required. `color` and `tid` are optional.

The first version provides only a C API. It must remain usable from C++ code in a compatible mode. A dedicated C++ wrapper is postponed.

A minimal set of convenience macros is required. It must also be evaluated whether the full API can be implemented as a header-only solution based on macros, without requiring an object file.

## Definition of Done
The task is considered complete if there is a dedicated hello-world test that uses RangeTap markers and passes compile and link checks.

After that, runtime validation must confirm that annotated regions appear in VTune and Nsight. The presence of those regions is the proof that the task is completed.

Required checks beyond compile and link:
- runtime smoke test under VTune and Nsight;
- nested push/pop validation;
- range begin/end validation.

After placing markers in their application, a user must be able to obtain an annotated profile in the selected profiler by changing a define, without rewriting the source code.

## Priorities
The top priorities at the start are minimal library weight and API simplicity.

The most critical first-step scope is a minimal API, VTune/Nsight support, and a hello-world test.

Intentionally postponed:
- ARM;
- Streamline;
- Fortran;
- a dedicated C++ wrapper;
- API expansion beyond the basic range markers.

## Risks and Unknowns
It is not yet fully clear how much functional overlap exists between different profilers and what the true common minimum API can be without losing too much useful capability.

No obvious blockers are visible at the moment.

The following decisions are still open:
- whether the implementation can be fully header-only;
- where the common API boundary lies between VTune/ITT and Nsight/NVTX;
- what exact minimal set of macros and functions will be included in the first version.

## First Step
The first step is to define the base project structure and place the public API, backend-specific adapters, examples, and tests into that structure. After that, the minimum set of functions and macros for the first working hello-world can be validated separately.

### Implementation Plan
1. Define the public C API and the list of compiler defines used to select a backend.
2. Separate the public layer from the backend-specific implementations so backend selection happens when compiling the target application.
3. Build a hello-world test that calls the `RNTP_` API and verify compile and link success.
4. Add runtime smoke tests for VTune and Nsight.
5. Write concise documentation and minimal usage examples.

### Proposed Directory Layout
```text
RangeTap/
├── CMakeLists.txt
├── TASK.md
├── README.md
├── cmake/
│   └── RangeTapConfig.cmake.in
├── include/
│   └── rangetap/
│       ├── rangetap.h
│       ├── rangetap_macros.h
│       └── rangetap_types.h
├── src/
│   ├── rangetap_common.c
│   ├── backend_itt.c
│   ├── backend_nvtx.c
│   └── backend_stub.c
├── examples/
│   ├── hello_vtune.c
│   ├── hello_nsight.c
│   └── nested_ranges.c
├── tests/
│   ├── compile/
│   │   ├── test_compile_vtune.c
│   │   ├── test_compile_nsight.c
│   │   └── test_cpp_compat.cpp
│   └── runtime/
│       ├── test_runtime_vtune.c
│       └── test_runtime_nsight.c
├── docs/
│   ├── api.md
│   └── backends.md
└── scripts/
    ├── run_vtune.sh
    └── run_nsight.sh
```

### Directory Responsibilities
- `include/rangetap/`: public headers and macros included by the user;
- `src/`: common implementation layer and backend-specific adapters;
- `examples/`: minimal examples for VTune and Nsight;
- `tests/compile/`: compile/link checks and C++ compatibility checks;
- `tests/runtime/`: smoke tests that verify visible regions in the profiler;
- `docs/`: concise API documentation and backend limitations;
- `scripts/`: local wrappers for manually running runtime checks.

### First Practical Step
Create `CMakeLists.txt`, `include/rangetap/rangetap.h`, and one compile/link hello-world test first. That gives a minimal skeleton to validate the initial API shape quickly.
