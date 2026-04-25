# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet resource library CMake file
#
################################################################################

# Packages #####################################################################
find_package(nlohmann_json REQUIRED)

# Meshes.
find_package(assimp REQUIRED)

# Textures.
find_package(Stb REQUIRED COMPONENTS stb_image)

# Shaders.
find_package(SPIRV-Tools-opt CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)

# Misc.
find_package(Threads REQUIRED)

# Source files #################################################################
target_sources(${EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/editor/asset/asset.h"
    "${PROJECT_SOURCE_DIR}/src/editor/asset/asset_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/editor/asset/asset_utils.cc"
    
    # Exporters.
    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/asset_exporter.cc"
    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/assimp_utils.cc"
    
    # Model.
    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/model/model_export.h"
    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/model/model_exporter.cc"
    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/model/model_export_label.cc"

    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/model/utils/animation_export_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/model/utils/model_exporter_utils.cc"
    
    # Shader.
    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/shader/shader_exporter.cc"
    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/shader/shader_module_exporter.cc"

    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/shader/data/shader_export_keys.h"

    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/shader/utils/shader_export_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/shader/utils/shader_module_export_utils.cc"
    
    # Texture.
    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/texture/data/texture_export_keys.h"

    "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/texture/texture_exporter.cc"
)

# Compiling ####################################################################
target_include_directories(${EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_libraries(${EXECUTABLE_NAME}
  PRIVATE
    ${COMET_LIBRARY_NAME}
    nlohmann_json::nlohmann_json
    assimp::assimp
    SPIRV-Tools-opt
    glslang::glslang
    unofficial::shaderc::shaderc
)
