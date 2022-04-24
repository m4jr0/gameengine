# Copyright 2022 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet Editor executable CMake file
#
################################################################################

# Source files #################################################################
list(APPEND EXECUTABLE_SOURCES
  "${PROJECT_SOURCE_DIR}/src/comet/entry_point.cc"
  "${PROJECT_SOURCE_DIR}/src/editor/editor.cc"
)

# Executable ###################################################################
add_executable(${EXECUTABLE_NAME} ${EXECUTABLE_SOURCES})

set_target_properties(${EXECUTABLE_NAME} PROPERTIES CXX_EXTENSIONS OFF)

# Global variables #############################################################
# Executables.
set(EXECUTABLE_NAME "comet_editor")
set(TESTS_EXECUTABLE_NAME "comet_tests")

# Internal library names.
set(ASSET_LIBRARY_NAME "asset")

# Internal libraries container.
list(APPEND COMET_EDITOR_LIBRARIES
  ${ASSET_LIBRARY_NAME}
)

# Subdirectories ###############################################################
add_subdirectory("${PROJECT_SOURCE_DIR}/src/editor/asset")

# Compiling ####################################################################
target_precompile_headers(${EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet_precompile.h"
)

target_include_directories(${EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    ${VCPKG_INCLUDE_DIR}
)

# Linking ######################################################################
target_link_directories(${EXECUTABLE_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${EXECUTABLE_NAME}
  PRIVATE
    ${COMET_LIBRARIES}
    ${COMET_EDITOR_LIBRARIES}
)