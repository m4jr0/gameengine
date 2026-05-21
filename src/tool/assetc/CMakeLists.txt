# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet asset compiler CMake file
#
################################################################################

add_executable(${COMET_ASSETC_TARGET})

comet_configure_target(${COMET_ASSETC_TARGET})

# Packages #####################################################################
find_package(nlohmann_json CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(SPIRV-Tools-opt CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)
find_package(Threads REQUIRED)

# Source files #################################################################
set(COMET_ASSETC_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/asset.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset_allocator.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset_allocator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset_processor.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset_processor.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cc"

  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/asset_exporter.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/asset_exporter.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/assimp_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/assimp_utils.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/model/model_export.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/model/model_export_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/model/model_export_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/model/model_exporter.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/model/model_exporter.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/model/utils/animation_export_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/model/utils/animation_export_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/model/utils/model_exporter_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/model/utils/model_exporter_utils.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/shader/data/shader_export_keys.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/shader/shader_exporter.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/shader/shader_exporter.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/shader/shader_module_exporter.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/shader/shader_module_exporter.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/shader/utils/shader_export_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/shader/utils/shader_export_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/shader/utils/shader_module_export_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/shader/utils/shader_module_export_utils.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/texture/data/texture_export_keys.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/texture/texture_exporter.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/exporter/texture/texture_exporter.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/memory/memory.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/memory/memory_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/memory/memory_label.h"
)

target_sources(${COMET_ASSETC_TARGET}
  PRIVATE
    ${COMET_ASSETC_SOURCES}
)

# Compiling ####################################################################
target_include_directories(${COMET_ASSETC_TARGET}
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${Stb_INCLUDE_DIR}"
)

# Linking ######################################################################
target_link_libraries(${COMET_ASSETC_TARGET}
  PRIVATE
    ${COMET_CORE_TARGET}
    ${COMET_DATA_TARGET}
    nlohmann_json::nlohmann_json
    assimp::assimp
    SPIRV-Tools-opt
    glslang::glslang
    unofficial::shaderc::shaderc
    Threads::Threads
)