# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet data library CMake file
#
################################################################################

add_library(${COMET_DATA_TARGET} STATIC)

comet_configure_target(${COMET_DATA_TARGET})

# Source files #################################################################
set(COMET_DATA_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/animation/animation_clip.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/animation/animation_id.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/animation/animation_set.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/animation/utils/animation_clip_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/geometry/mesh.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/geometry/skeleton.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/input/input.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/input/input_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/light/light.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/light/light_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/light/light_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/render/label/pipeline_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/render/label/shader_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/render/label/texture_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/render/pipeline.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/render/shader.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/render/texture.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/render/utils/shader_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/render/utils/texture_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/resource/animation/animation_resource.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/resource/common.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/resource/label/common_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/resource/material/material_resource.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/resource/model/model_resource.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/resource/resource.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/resource/resource_file.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/resource/resource_id.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/resource/shader/shader_module_resource.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/resource/shader/shader_resource.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/data/resource/texture/texture_resource.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/animation/animation_id.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/animation/animation_set.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/animation/utils/animation_clip_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/input/input_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/light/light_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/light/light_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/render/label/pipeline_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/render/label/shader_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/render/label/texture_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/render/utils/shader_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/render/utils/texture_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/animation/animation_resource.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/label/common_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/material/material_resource.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/model/model_resource.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/resource_file.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/shader/shader_module_resource.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/shader/shader_resource.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/texture/texture_resource.cc"
)

target_sources(${COMET_DATA_TARGET}
  PRIVATE
    ${COMET_DATA_SOURCES}
)

# Compiling ####################################################################
target_include_directories(${COMET_DATA_TARGET}
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Precompiled headers ##########################################################
target_precompile_headers(${COMET_DATA_TARGET}
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/comet_data_pch.h"
)

# Linking ######################################################################
target_link_libraries(${COMET_DATA_TARGET}
  PUBLIC
    ${COMET_CORE_TARGET}
)