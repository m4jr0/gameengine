# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet engine CMake file
#
################################################################################

# Subdirectories ###############################################################
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/core")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/data")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/runtime")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/platform")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/render")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/engine")

# Public Comet target ##########################################################
add_library(${COMET_TARGET} INTERFACE)

target_include_directories(${COMET_TARGET}
  INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/build/include"
)

target_sources(${COMET_TARGET}
  INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/build/include/comet/build/build_define.h"
)

target_link_libraries(${COMET_TARGET}
  INTERFACE
    ${COMET_ENGINE_TARGET}
)