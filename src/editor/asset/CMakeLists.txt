# Copyright 2022 m4jr0. All Rights Reserved.
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
find_package(unofficial-shaderc_util CONFIG REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)

# Misc.
find_package(Threads REQUIRED)

# Source files #################################################################
add_library(${ASSET_LIBRARY_NAME}
  "${PROJECT_SOURCE_DIR}/src/editor/asset/asset.h"
  "${PROJECT_SOURCE_DIR}/src/editor/asset/asset_manager.cc"
  "${PROJECT_SOURCE_DIR}/src/editor/asset/asset_utils.cc"

  # Exporters.
  "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/asset_exporter.cc"
  "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/model_exporter.cc"
  "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/shader_exporter.cc"
  "${PROJECT_SOURCE_DIR}/src/editor/asset/exporter/texture_exporter.cc"
)

# Compiling ####################################################################
target_include_directories(${ASSET_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_directories(${ASSET_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
    target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)
)

target_link_libraries(${ASSET_LIBRARY_NAME}
  PRIVATE
    assimp::assimp
    SPIRV-Tools-opt
    glslang
    unofficial::shaderc_util::shaderc_util
    unofficial::shaderc::shaderc
)

if(WIN32)
  target_link_libraries(${ASSET_LIBRARY_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${ASSET_LIBRARY_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()