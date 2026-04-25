# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet animation library CMake file
#
################################################################################

# Source files #################################################################
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet/animation/animation_id.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/animation/animation_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/animation/animation_set.cc"
    
    # Component.
    "${PROJECT_SOURCE_DIR}/src/comet/animation/component/animation_component.h"

    # Type.
    "${PROJECT_SOURCE_DIR}/src/comet/animation/type/animation_clip.h"
    "${PROJECT_SOURCE_DIR}/src/comet/animation/type/animation_pose.h"
    "${PROJECT_SOURCE_DIR}/src/comet/animation/type/animation_skinning.h"

    # Utils.
    "${PROJECT_SOURCE_DIR}/src/comet/animation/utils/animation_clip_utils.cc"
)

# Compiling ####################################################################
target_include_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/"
)
