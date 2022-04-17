# Copyright 2022 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet tests executable CMake file
#
################################################################################

# Source files #################################################################
list(APPEND TESTS_EXECUTABLE_SOURCES
  "${PROJECT_SOURCE_DIR}/src/tests/tests.cc"

  "${PROJECT_SOURCE_DIR}/src/tests/dummies/dummy_object.cc"

  "${PROJECT_SOURCE_DIR}/src/tests/core/tests_engine.cc"

  "${PROJECT_SOURCE_DIR}/src/tests/game_object/tests_game_object.cc"

  "${PROJECT_SOURCE_DIR}/src/tests/utils/tests_file_system.cc"

  "${PROJECT_SOURCE_DIR}/src/tests/utils/structure/tests_ring_queue.cc"
)

# Executable ###################################################################
add_executable(${TESTS_EXECUTABLE_NAME} ${TESTS_EXECUTABLE_SOURCES})

# Compiling ####################################################################
target_precompile_headers(${TESTS_EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet_precompile.h"
)

target_include_directories(${TESTS_EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    ${VCPKG_INCLUDE_DIR}
)

# Linking ######################################################################
target_link_directories(${TESTS_EXECUTABLE_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${TESTS_EXECUTABLE_NAME}
  PRIVATE
    ${COMET_LIBRARIES}
)
