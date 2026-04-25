# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet input library CMake file
#
################################################################################

# Source files #################################################################
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet/input/input_event.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/input/input_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/input/input_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/input/input.h"
)

# Compiling ####################################################################
target_include_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/"
)
