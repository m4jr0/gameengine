# Copyright 2025 m4jr0. All Rights Reserved.
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
    "${PROJECT_SOURCE_DIR}/src/comet/geometry/geometry_common.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/geometry/geometry_manager.cc"

    # Components.
    "${PROJECT_SOURCE_DIR}/src/comet/geometry/component/mesh_component.h"
    "${PROJECT_SOURCE_DIR}/src/comet/geometry/component/model_component.h"
    "${PROJECT_SOURCE_DIR}/src/comet/geometry/component/skeleton_component.h"
)

# Compiling ####################################################################
target_include_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${COMET_LIBRARY_NAME}
  PRIVATE
    # Put libraries here.
)

if(WIN32)
  target_link_libraries(${COMET_LIBRARY_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${COMET_LIBRARY_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()
