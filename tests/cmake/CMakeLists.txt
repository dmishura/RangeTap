set(RNTP_CMAKE_TEST_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/consumer")
set(RNTP_MOCK_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/mock/include")
set(RNTP_MOCK_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/mock/lib/libprofiler.a")
function(rntp_add_configure_test name backend)
    add_test(NAME cmake_${name} COMMAND "${CMAKE_COMMAND}"
        -S "${RNTP_CMAKE_TEST_SOURCE}" -B "${CMAKE_CURRENT_BINARY_DIR}/${name}"
        -DRANGETAP_SOURCE=${PROJECT_SOURCE_DIR} -DEXPECTED_BACKEND=${backend}
        -DRNTP_MOCK_INCLUDE=${RNTP_MOCK_INCLUDE} -DRNTP_MOCK_LIBRARY=${RNTP_MOCK_LIBRARY}
        -DRNTP_BACKEND=${backend} -DRNTP_NVTX_INCLUDE_DIR=${RNTP_MOCK_INCLUDE}
        -DRNTP_ITT_INCLUDE_DIR=${RNTP_MOCK_INCLUDE} -DRNTP_ITT_LIBRARY=${RNTP_MOCK_LIBRARY}
        -DRNTP_STREAMLINE_INCLUDE_DIR=${RNTP_MOCK_INCLUDE} -DRNTP_STREAMLINE_LIBRARY=${RNTP_MOCK_LIBRARY})
    add_test(NAME cmake_${name}_build
        COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}/${name}")
    set_tests_properties(cmake_${name}_build PROPERTIES DEPENDS cmake_${name})
endfunction()
add_test(NAME cmake_default COMMAND "${CMAKE_COMMAND}"
    -S "${RNTP_CMAKE_TEST_SOURCE}" -B "${CMAKE_CURRENT_BINARY_DIR}/default"
    -DRANGETAP_SOURCE=${PROJECT_SOURCE_DIR} -DEXPECTED_BACKEND=NONE
    -DRNTP_MOCK_INCLUDE=${RNTP_MOCK_INCLUDE} -DRNTP_MOCK_LIBRARY=${RNTP_MOCK_LIBRARY})
add_test(NAME cmake_default_build
    COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}/default")
set_tests_properties(cmake_default_build PROPERTIES DEPENDS cmake_default)
rntp_add_configure_test(nvtx NVTX)
rntp_add_configure_test(lowercase_nvtx nvtx)
rntp_add_configure_test(itt ITT)
rntp_add_configure_test(streamline STREAMLINE)
add_test(NAME cmake_invalid_backend COMMAND "${CMAKE_COMMAND}"
    -S "${RNTP_CMAKE_TEST_SOURCE}" -B "${CMAKE_CURRENT_BINARY_DIR}/invalid"
    -DRANGETAP_SOURCE=${PROJECT_SOURCE_DIR} -DEXPECTED_BACKEND=INVALID -DRNTP_BACKEND=INVALID)
set_tests_properties(cmake_invalid_backend PROPERTIES WILL_FAIL TRUE)
