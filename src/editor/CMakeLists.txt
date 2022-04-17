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
  "${PROJECT_SOURCE_DIR}/src/editor/editor.cc"
)

# Executable ###################################################################
add_executable(${EXECUTABLE_NAME} ${EXECUTABLE_SOURCES})

# Compiling ####################################################################
target_precompile_headers(${EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet_precompile.h"
    "${PROJECT_SOURCE_DIR}/src/debug_windows.h"
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
)
