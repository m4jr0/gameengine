# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet engine CMake file
#
################################################################################

# Subdirectories ###############################################################
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/core")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/data")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/runtime")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/platform")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/render")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/engine")

add_library(${COMET_TARGET} INTERFACE)

target_link_libraries(${COMET_TARGET}
  INTERFACE
    ${COMET_ENGINE_TARGET}
)
