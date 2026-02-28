# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet resource library CMake file
#
################################################################################

# Source files #################################################################
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet/resource/animation_resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/material_resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/model_resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/resource_allocator.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/resource_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/shader_module_resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/shader_resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/texture_resource.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/resource/handler/animation_resource_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/handler/material_resource_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/handler/model_resource_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/handler/resource_handler.h"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/handler/resource_handler_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/handler/shader_module_resource_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/handler/shader_resource_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/handler/texture_resource_handler.cc"
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
    # Put internal libraries here.
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
