# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet tests executable CMake file
#
################################################################################

# Packages #####################################################################
find_package(Catch2 3 REQUIRED)

# Names ########################################################################
set(FOUNDATION_TESTS_EXECUTABLE_NAME "comet_foundation_tests")
set(ENGINE_TESTS_EXECUTABLE_NAME "comet_engine_tests")

# Source files #################################################################
set(FOUNDATION_TESTS_EXECUTABLE_SOURCES
  "${PROJECT_SOURCE_DIR}/src/tests/dummies/dummy_object.cc"
  
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/tests_main_foundation.cc"

  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_array.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_bitset.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_hash_set.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_map.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_ring_queue.cc"
  "${PROJECT_SOURCE_DIR}/src/tests/foundation/core/type/tests_tstring.cc"
)

set(ENGINE_TESTS_EXECUTABLE_SOURCES
  "${PROJECT_SOURCE_DIR}/src/tests/dummies/dummy_object.cc"

  "${PROJECT_SOURCE_DIR}/src/tests/engine/tests_main_engine.cc"

  "${PROJECT_SOURCE_DIR}/src/tests/engine/core/tests_file_system.cc"

  "${PROJECT_SOURCE_DIR}/src/tests/engine/entity/tests_entity.cc"
)

# Executables ##################################################################
add_executable(${FOUNDATION_TESTS_EXECUTABLE_NAME}
  ${FOUNDATION_TESTS_EXECUTABLE_SOURCES}
)

add_executable(${ENGINE_TESTS_EXECUTABLE_NAME}
  ${ENGINE_TESTS_EXECUTABLE_SOURCES}
)

set_target_properties(${FOUNDATION_TESTS_EXECUTABLE_NAME}
  PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_RTTI OFF
)

set_target_properties(${ENGINE_TESTS_EXECUTABLE_NAME}
  PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_RTTI OFF
)

# Compiling ####################################################################
target_include_directories(${FOUNDATION_TESTS_EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    ${PICOSHA2_INCLUDE_DIRS}
)

target_include_directories(${ENGINE_TESTS_EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    ${PICOSHA2_INCLUDE_DIRS}
)

# Precompiled headers ##########################################################
target_precompile_headers(${ENGINE_TESTS_EXECUTABLE_NAME}
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/tests_pch.h"
)

# Linking ######################################################################
target_link_libraries(${FOUNDATION_TESTS_EXECUTABLE_NAME}
  PRIVATE
    Catch2::Catch2WithMain
    ${COMET_LIBRARY_NAME}
)

target_link_libraries(${ENGINE_TESTS_EXECUTABLE_NAME}
  PRIVATE
    Catch2::Catch2WithMain
    ${COMET_LIBRARY_NAME}
)
