# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

find_package(Catch2 3 CONFIG REQUIRED)

set(FOUNDATION_TESTS_EXECUTABLE_NAME "comet_foundation_tests")
set(FOUNDATION_TESTS_EXECUTABLE_SOURCES
  "${PROJECT_SOURCE_DIR}/src/tests/dummies/dummy_object.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/tests_main_foundation.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_command_line.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_c_string_format.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_array.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_bitset.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_hash_set.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_map.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_ring_queue.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_tstring.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_system_allocator.cc"
)

add_executable(${FOUNDATION_TESTS_EXECUTABLE_NAME}
  ${FOUNDATION_TESTS_EXECUTABLE_SOURCES}
)
target_include_directories(${FOUNDATION_TESTS_EXECUTABLE_NAME}
  PRIVATE "${PROJECT_SOURCE_DIR}/src/tests"
          "${PROJECT_SOURCE_DIR}/src"
)
target_link_libraries(${FOUNDATION_TESTS_EXECUTABLE_NAME}
  PRIVATE Catch2::Catch2WithMain ${COMET_CORE_TARGET}
)
add_test(NAME comet_foundation_tests COMMAND ${FOUNDATION_TESTS_EXECUTABLE_NAME})
add_dependencies(${FOUNDATION_TESTS_EXECUTABLE_NAME} comet_check_dependencies)

if(NOT COMET_BUILD_FOUNDATION_ONLY)
  set(ENGINE_TESTS_EXECUTABLE_NAME "comet_engine_tests")
  add_executable(${ENGINE_TESTS_EXECUTABLE_NAME}
    "${PROJECT_SOURCE_DIR}/src/tests/dummies/dummy_object.cc"
    "${PROJECT_SOURCE_DIR}/src/tests/engine/tests_main_engine.cc"
    "${PROJECT_SOURCE_DIR}/src/tests/engine/core/tests_file_system.cc"
    "${PROJECT_SOURCE_DIR}/src/tests/engine/entity/tests_entity.cc"
  )
  target_include_directories(${ENGINE_TESTS_EXECUTABLE_NAME}
    PRIVATE "${PROJECT_SOURCE_DIR}/src/tests"
            "${PROJECT_SOURCE_DIR}/src"
  )
  target_precompile_headers(${ENGINE_TESTS_EXECUTABLE_NAME}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests_pch.h"
  )
  target_link_libraries(${ENGINE_TESTS_EXECUTABLE_NAME}
    PRIVATE Catch2::Catch2WithMain ${COMET_ENGINE_TARGET}
  )
  add_test(NAME comet_engine_tests COMMAND ${ENGINE_TESTS_EXECUTABLE_NAME})
endif()
