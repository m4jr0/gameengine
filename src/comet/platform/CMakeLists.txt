# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet platform library CMake file
#
################################################################################

add_library(${COMET_PLATFORM_TARGET} STATIC)

comet_configure_target(${COMET_PLATFORM_TARGET})

# Packages #####################################################################
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

# Source files #################################################################
set(COMET_PLATFORM_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/platform.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/platform/input/glfw_input_backend.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/platform/window/glfw/empty/empty_glfw_window.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/platform/window/glfw/glfw_window.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/platform/window/glfw/opengl/opengl_glfw_window.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/platform/window/glfw/vulkan/vulkan_glfw_window.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/platform/window/window.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/platform/window/window_common.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/platform/window/window_event.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/input/glfw_input_backend.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/window/glfw/empty/empty_glfw_window.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/window/glfw/glfw_window.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/window/glfw/opengl/opengl_glfw_window.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/window/glfw/vulkan/vulkan_glfw_window.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/window/window.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/window/window_event.cc"
)

target_sources(${COMET_PLATFORM_TARGET}
  PRIVATE
    ${COMET_PLATFORM_SOURCES}
)

# Compiling ####################################################################
target_include_directories(${COMET_PLATFORM_TARGET}
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Precompiled headers ##########################################################
target_precompile_headers(${COMET_PLATFORM_TARGET}
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/comet_platform_pch.h"
)

# Linking ######################################################################
target_link_libraries(${COMET_PLATFORM_TARGET}
  PUBLIC
    ${COMET_CORE_TARGET}
    ${COMET_RUNTIME_TARGET}
    glfw
    glad::glad
    Vulkan::Vulkan
)