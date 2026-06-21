# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet sandbox CMake file
#
################################################################################

add_executable(${COMET_SANDBOX_TARGET})

comet_configure_target(${COMET_SANDBOX_TARGET})

# Source files #################################################################
set(COMET_SANDBOX_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/camera_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/camera_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/sandbox.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/sandbox.h"
)

target_sources(${COMET_SANDBOX_TARGET}
  PRIVATE
    ${COMET_SANDBOX_SOURCES}
)

# Compiling ####################################################################
target_include_directories(${COMET_SANDBOX_TARGET}
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Precompiled headers ##########################################################
target_precompile_headers(${COMET_SANDBOX_TARGET}
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/sandbox_pch.h"
)

# Linking ######################################################################
target_link_libraries(${COMET_SANDBOX_TARGET}
  PRIVATE
    ${COMET_TARGET}
)