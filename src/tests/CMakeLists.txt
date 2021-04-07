# Comet tests executable #######################################################
list(APPEND TESTS_EXECUTABLE_SOURCES
  "${PROJECT_SOURCE_DIR}/src/tests/dummies.cpp"
  "${PROJECT_SOURCE_DIR}/src/tests/tests.cpp"
  "${PROJECT_SOURCE_DIR}/src/tests/tests_engine.cpp"
  "${PROJECT_SOURCE_DIR}/src/tests/tests_file_system.cpp"
  "${PROJECT_SOURCE_DIR}/src/tests/tests_ring_queue.cpp"
)

add_executable(${TESTS_EXECUTABLE_NAME} ${TESTS_EXECUTABLE_SOURCES})

target_precompile_headers(${TESTS_EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet_precompile.hpp"
)

target_include_directories(${TESTS_EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    ${VCPKG_INCLUDE_DIR}
)

target_link_directories(${TESTS_EXECUTABLE_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${TESTS_EXECUTABLE_NAME}
  PRIVATE
    ${COMET_LIBRARIES}
)
