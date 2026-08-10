cmake_minimum_required(VERSION 3.20)

project(RangeTap VERSION 0.1.0 LANGUAGES C CXX)

add_library(RangeTap INTERFACE)
add_library(RangeTap::RangeTap ALIAS RangeTap)

target_include_directories(RangeTap
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(RangeTap INTERFACE c_std_99)

option(RNTP_BUILD_TESTS "Build RangeTap compile tests" ON)

if(RNTP_BUILD_TESTS)
    enable_testing()

    file(GLOB RNTP_NVTX_ROOT_HINTS LIST_DIRECTORIES true
        "/opt/nvidia/nsight-systems-cli/*/target-linux-x64/nvtx"
        "/opt/nvidia/nsight-systems/*/target-linux-x64/nvtx"
        "/opt/nvidia/nsight-compute/*/host/target-linux-x64/nvtx"
    )
    find_path(RNTP_NVTX_INCLUDE_DIR
        NAMES nvtx3/nvToolsExt.h
        HINTS ${RNTP_NVTX_ROOT_HINTS}
        PATH_SUFFIXES include
        DOC "Directory containing the NVTX headers"
    )

    file(GLOB RNTP_ITT_ROOT_HINTS LIST_DIRECTORIES true
        "/opt/intel/oneapi/vtune/*"
    )
    find_path(RNTP_ITT_INCLUDE_DIR
        NAMES ittnotify.h
        HINTS ${RNTP_ITT_ROOT_HINTS}
        PATH_SUFFIXES include sdk/include
        DOC "Directory containing the Intel ITT headers"
    )
    find_library(RNTP_ITT_LIBRARY
        NAMES ittnotify
        HINTS ${RNTP_ITT_ROOT_HINTS}
        PATH_SUFFIXES lib64 sdk/lib64
        DOC "Intel ITT static or shared library"
    )

    add_executable(rntp_test_none tests/compile/test_compile_none.c)
    target_link_libraries(rntp_test_none PRIVATE RangeTap::RangeTap)
    add_test(NAME rntp_test_none COMMAND rntp_test_none)

    if(RNTP_NVTX_INCLUDE_DIR)
        add_executable(rntp_test_nvtx tests/compile/test_compile_nvtx.c)
        target_link_libraries(rntp_test_nvtx PRIVATE RangeTap::RangeTap)
        target_compile_definitions(rntp_test_nvtx PRIVATE RNTP_BACKEND=RNTP_BACKEND_NVTX)
        target_include_directories(rntp_test_nvtx PRIVATE ${RNTP_NVTX_INCLUDE_DIR})
        add_test(NAME rntp_test_nvtx COMMAND rntp_test_nvtx)

        add_executable(rntp_runtime_nvtx tests/runtime/test_runtime.c)
        target_link_libraries(rntp_runtime_nvtx PRIVATE RangeTap::RangeTap)
        target_compile_definitions(rntp_runtime_nvtx PRIVATE RNTP_BACKEND=RNTP_BACKEND_NVTX)
        target_include_directories(rntp_runtime_nvtx PRIVATE ${RNTP_NVTX_INCLUDE_DIR})
        add_test(NAME rntp_runtime_nvtx COMMAND rntp_runtime_nvtx)
    else()
        message(STATUS "NVTX headers not found; skipping the NVTX compile test")
    endif()

    if(RNTP_ITT_INCLUDE_DIR AND RNTP_ITT_LIBRARY)
        add_executable(rntp_test_itt tests/compile/test_compile_itt.c)
        target_link_libraries(rntp_test_itt PRIVATE RangeTap::RangeTap ${RNTP_ITT_LIBRARY} ${CMAKE_DL_LIBS})
        target_compile_definitions(rntp_test_itt PRIVATE RNTP_BACKEND=RNTP_BACKEND_ITT)
        target_include_directories(rntp_test_itt PRIVATE ${RNTP_ITT_INCLUDE_DIR})
        add_test(NAME rntp_test_itt COMMAND rntp_test_itt)

        add_executable(rntp_runtime_itt tests/runtime/test_runtime.c)
        target_link_libraries(rntp_runtime_itt PRIVATE RangeTap::RangeTap ${RNTP_ITT_LIBRARY} ${CMAKE_DL_LIBS})
        target_compile_definitions(rntp_runtime_itt PRIVATE RNTP_BACKEND=RNTP_BACKEND_ITT)
        target_include_directories(rntp_runtime_itt PRIVATE ${RNTP_ITT_INCLUDE_DIR})
        add_test(NAME rntp_runtime_itt COMMAND rntp_runtime_itt)
    else()
        message(STATUS "Intel ITT SDK not found; skipping the ITT compile test")
    endif()
endif()
