cmake_minimum_required(VERSION 3.20)

project(RangeTap VERSION 0.1.0 LANGUAGES C CXX)

add_library(RangeTap INTERFACE)
add_library(RangeTap::RangeTap ALIAS RangeTap)

target_include_directories(RangeTap
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(RangeTap INTERFACE c_std_99)

function(rangetap_link_backend target backend)
    if(NOT TARGET "${target}")
        message(FATAL_ERROR "RangeTap: target '${target}' does not exist")
    endif()

    string(TOUPPER "${backend}" backend)
    if(NOT backend MATCHES "^(NONE|NVTX|ITT|STREAMLINE)$")
        message(FATAL_ERROR "RangeTap: unsupported backend '${backend}'")
    endif()

    get_target_property(RNTP_CURRENT_BACKEND "${target}" RNTP_BACKEND)
    if(RNTP_CURRENT_BACKEND)
        if(NOT RNTP_CURRENT_BACKEND STREQUAL backend)
            message(FATAL_ERROR
                "RangeTap: target '${target}' already uses the ${RNTP_CURRENT_BACKEND} backend"
            )
        endif()
        return()
    endif()

    get_target_property(RNTP_TARGET_TYPE "${target}" TYPE)
    if(RNTP_TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
        set(RNTP_LINK_SCOPE INTERFACE)
    else()
        set(RNTP_LINK_SCOPE PRIVATE)
    endif()

    target_link_libraries("${target}" ${RNTP_LINK_SCOPE} RangeTap::RangeTap)
    target_compile_definitions("${target}" ${RNTP_LINK_SCOPE}
        RNTP_BACKEND=RNTP_BACKEND_${backend}
    )

    if(backend STREQUAL "NVTX")
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
        if(NOT RNTP_NVTX_INCLUDE_DIR)
            message(FATAL_ERROR
                "RangeTap: NVTX headers not found; set RNTP_NVTX_INCLUDE_DIR"
            )
        endif()
        target_include_directories("${target}" ${RNTP_LINK_SCOPE} ${RNTP_NVTX_INCLUDE_DIR})

    elseif(backend STREQUAL "ITT")
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
        if(NOT RNTP_ITT_INCLUDE_DIR OR NOT RNTP_ITT_LIBRARY)
            message(FATAL_ERROR
                "RangeTap: Intel ITT SDK not found; set RNTP_ITT_INCLUDE_DIR and RNTP_ITT_LIBRARY"
            )
        endif()
        target_include_directories("${target}" ${RNTP_LINK_SCOPE} ${RNTP_ITT_INCLUDE_DIR})
        target_link_libraries("${target}" ${RNTP_LINK_SCOPE} ${RNTP_ITT_LIBRARY} ${CMAKE_DL_LIBS})

    elseif(backend STREQUAL "STREAMLINE")
        find_path(RNTP_STREAMLINE_INCLUDE_DIR
            NAMES streamline_annotate.h
            HINTS "$ENV{HOME}/gator/annotate"
            DOC "Directory containing the Arm Streamline annotation header"
        )
        find_library(RNTP_STREAMLINE_LIBRARY
            NAMES streamline_annotate
            HINTS "$ENV{HOME}/gator/annotate"
            DOC "Arm Streamline annotation library"
        )
        if(NOT RNTP_STREAMLINE_INCLUDE_DIR OR NOT RNTP_STREAMLINE_LIBRARY)
            message(FATAL_ERROR
                "RangeTap: Arm Streamline annotations not found; set RNTP_STREAMLINE_INCLUDE_DIR and RNTP_STREAMLINE_LIBRARY"
            )
        endif()
        find_package(Threads REQUIRED)
        target_include_directories("${target}" ${RNTP_LINK_SCOPE} ${RNTP_STREAMLINE_INCLUDE_DIR})
        target_link_libraries("${target}" ${RNTP_LINK_SCOPE} ${RNTP_STREAMLINE_LIBRARY} Threads::Threads)
    endif()

    set_property(TARGET "${target}" PROPERTY RNTP_BACKEND "${backend}")
endfunction()

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(RNTP_BUILD_TESTS_DEFAULT ON)
else()
    set(RNTP_BUILD_TESTS_DEFAULT OFF)
endif()
option(RNTP_BUILD_TESTS "Build RangeTap compile tests" ${RNTP_BUILD_TESTS_DEFAULT})

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

    find_path(RNTP_STREAMLINE_INCLUDE_DIR
        NAMES streamline_annotate.h
        HINTS "$ENV{HOME}/gator/annotate"
        DOC "Directory containing the Arm Streamline annotation header"
    )
    find_library(RNTP_STREAMLINE_LIBRARY
        NAMES streamline_annotate
        HINTS "$ENV{HOME}/gator/annotate"
        DOC "Arm Streamline annotation library"
    )

    add_executable(rntp_test_none tests/compile/test_compile_none.c)
    rangetap_link_backend(rntp_test_none NONE)
    add_test(NAME rntp_test_none COMMAND rntp_test_none)

    if(RNTP_NVTX_INCLUDE_DIR)
        add_executable(rntp_test_nvtx tests/compile/test_compile_nvtx.c)
        rangetap_link_backend(rntp_test_nvtx NVTX)
        add_test(NAME rntp_test_nvtx COMMAND rntp_test_nvtx)

        add_executable(rntp_runtime_nvtx tests/runtime/test_runtime.c)
        rangetap_link_backend(rntp_runtime_nvtx NVTX)
        add_test(NAME rntp_runtime_nvtx COMMAND rntp_runtime_nvtx)
    else()
        message(STATUS "NVTX headers not found; skipping the NVTX compile test")
    endif()

    if(RNTP_ITT_INCLUDE_DIR AND RNTP_ITT_LIBRARY)
        add_executable(rntp_test_itt tests/compile/test_compile_itt.c)
        rangetap_link_backend(rntp_test_itt ITT)
        add_test(NAME rntp_test_itt COMMAND rntp_test_itt)

        add_executable(rntp_runtime_itt tests/runtime/test_runtime.c)
        rangetap_link_backend(rntp_runtime_itt ITT)
        add_test(NAME rntp_runtime_itt COMMAND rntp_runtime_itt)
    else()
        message(STATUS "Intel ITT SDK not found; skipping the ITT compile test")
    endif()

    if(RNTP_STREAMLINE_INCLUDE_DIR AND RNTP_STREAMLINE_LIBRARY)
        add_executable(rntp_test_streamline tests/compile/test_compile_streamline.c)
        rangetap_link_backend(rntp_test_streamline STREAMLINE)
    else()
        message(STATUS "Arm Streamline annotations not found; skipping the Streamline compile/link test")
    endif()
endif()
