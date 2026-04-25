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
    "${PROJECT_SOURCE_DIR}/src/comet/resource/resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/resource_allocator.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/resource_id.h"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/resource_manager.cc"

    # Handler.
    "${PROJECT_SOURCE_DIR}/src/comet/resource/handler/resource_handler.h"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/handler/resource_handler_utils.cc"

    # Label.
    "${PROJECT_SOURCE_DIR}/src/comet/resource/label/common_label.cc"

    # Runtime.
    "${PROJECT_SOURCE_DIR}/src/comet/resource/runtime/loaded_resource_handle.h"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/runtime/resource_slot.h"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/runtime/resource_slots.h"
    
    # Type.
    "${PROJECT_SOURCE_DIR}/src/comet/resource/type/common.h"

    # Animation.
    "${PROJECT_SOURCE_DIR}/src/comet/resource/animation/animation_resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/animation/animation_resource_handler.cc"

    # Material.
    "${PROJECT_SOURCE_DIR}/src/comet/resource/material/material_resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/material/material_resource_handler.cc"

    # Model.
    "${PROJECT_SOURCE_DIR}/src/comet/resource/model/model_resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/model/model_resource_handler.cc"

    # Shader.
    "${PROJECT_SOURCE_DIR}/src/comet/resource/shader/shader_module_resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/shader/shader_module_resource_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/shader/shader_resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/shader/shader_resource_handler.cc"

    # Texture.
    "${PROJECT_SOURCE_DIR}/src/comet/resource/texture/texture_resource.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/resource/texture/texture_resource_handler.cc"
)

# Compiling ####################################################################
target_include_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/"
)
