# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet geometry library CMake file
#
################################################################################

# Source files #################################################################
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet/geometry/geometry_manager.cc"

    # Component.
    "${PROJECT_SOURCE_DIR}/src/comet/geometry/component/mesh_component.h"
    "${PROJECT_SOURCE_DIR}/src/comet/geometry/component/model_component.h"
    "${PROJECT_SOURCE_DIR}/src/comet/geometry/component/skeleton_component.h"

    # Label.
    "${PROJECT_SOURCE_DIR}/src/comet/geometry/label/mesh_label.cc"

    # Type.
    "${PROJECT_SOURCE_DIR}/src/comet/geometry/type/mesh.h"
    "${PROJECT_SOURCE_DIR}/src/comet/geometry/type/skeleton.h"
)

# Compiling ####################################################################
target_include_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/"
)
