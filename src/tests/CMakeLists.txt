# Copyright 2024 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet tests executable CMake file
#
################################################################################

# Packages #####################################################################
find_package(Catch2 3 REQUIRED)

# Source files #################################################################
list(APPEND TESTS_EXECUTABLE_SOURCES
  "${PROJECT_SOURCE_DIR}/src/tests/tests.cc"

  "${PROJECT_SOURCE_DIR}/src/tests/dummies/dummy_object.cc"

  "${PROJECT_SOURCE_DIR}/src/tests/entity/tests_entity.cc"

  "${PROJECT_SOURCE_DIR}/src/tests/core/tests_file_system.cc"

  "${PROJECT_SOURCE_DIR}/src/tests/core/type/tests_ring_queue.cc"
)

# Executable ###################################################################
add_executable(${TESTS_EXECUTABLE_NAME} ${TESTS_EXECUTABLE_SOURCES})

set_target_properties(${TESTS_EXECUTABLE_NAME} PROPERTIES CXX_EXTENSIONS OFF CXX_RTTI OFF)

# Compiling ####################################################################
target_precompile_headers(${TESTS_EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet_precompile.h"
)

target_include_directories(${TESTS_EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    ${VCPKG_INCLUDE_DIR}
    ${PICOSHA2_INCLUDE_DIRS}
)

# Linking ######################################################################
target_link_directories(${TESTS_EXECUTABLE_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${TESTS_EXECUTABLE_NAME}
  PRIVATE
    Catch2::Catch2WithMain
    ${COMET_LIBRARY_NAME}
)