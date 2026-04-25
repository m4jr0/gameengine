# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet rendering library CMake file
#
################################################################################

# Packages #####################################################################
# GLFW.
find_package(glfw3 CONFIG REQUIRED)

# OpenGL.
find_package(glad CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# Vulkan.
find_package(Vulkan REQUIRED)

if(NOT WIN32)
  find_package(VulkanMemoryAllocator CONFIG REQUIRED)
endif()

# ImGui.
find_package(imgui CONFIG REQUIRED)

# Z-Library.
find_package(ZLIB REQUIRED)

# Source files #################################################################
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
    ${COMET_VMA_SOURCE}
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/camera.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/camera_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/debug_ui_registry.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/frustum.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/light_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/rendering_handle.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/rendering_manager.cc"

    # Label.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/label/common_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/label/light_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/label/pipeline_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/label/shader_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/label/texture_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/label/view_label.cc"

    # Type.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/camera.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/common.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/light.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/pipeline.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/shader.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/texture.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/view.h"

    # Utils.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/camera_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/culling_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/driver_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/geometry_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/light_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/shader_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/texture_utils.cc"

    # Windows.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/glfw/glfw_window.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/glfw/empty/empty_glfw_window.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/glfw/opengl/opengl_glfw_window.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/glfw/vulkan/vulkan_glfw_window.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/window.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/window_event.cc"

    # Driver.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/driver.cc"

    # OpenGL.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/opengl_debug.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/opengl_driver.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/opengl_frame_state.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_lighting_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_material_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_mesh_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_render_proxy_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_sampler_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_shader_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_shader_module_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_texture_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_view_handler.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/label/opengl_shader_module_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/label/opengl_texture_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/label/opengl_view_label.cc"
    
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_frame.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_framebuffer.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_light.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_material.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_mesh.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_pipeline.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_region_gpu_buffer.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_render_proxy.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_sampler.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_shader.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_shader_module.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_shadow.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_storage.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_texture.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_texture_map.h"
    
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/utils/opengl_sampler_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/utils/opengl_shader_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/utils/opengl_texture_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/utils/opengl_texture_map_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/utils/opengl_view_shader_utils.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/view/opengl_debug_view.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/view/opengl_imgui_view.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/view/opengl_shadow_view.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/view/opengl_view.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/view/opengl_world_view.cc"

    # Vulkan.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_alloc.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_context.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_debug.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_device.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_driver.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_swapchain.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_vma.cc"
    
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_descriptor_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_lighting_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_material_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_mesh_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_pipeline_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_render_pass_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_render_proxy_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_sampler_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_shader_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_shader_module_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_texture_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_view_handler.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/label/vulkan_descriptor_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/label/vulkan_pipeline_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/label/vulkan_render_pass_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/label/vulkan_texture_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/label/vulkan_view_label.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_buffer.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_command_buffer.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_descriptor.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_frame.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_image.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_light.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_material.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_mesh.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_pipeline.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_region_gpu_buffer.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_render_pass.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_render_proxy.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_sampler.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_shader.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_shader_module.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_texture.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_texture_map.h"

    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_descriptor_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_image_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_render_pass_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_sampler_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_shader_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_texture_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_texture_map_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_view_shader_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_view_utils.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/view/vulkan_debug_view.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/view/vulkan_imgui_view.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/view/vulkan_shadow_view.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/view/vulkan_view.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/view/vulkan_world_view.cc"

    # Empty.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/empty/empty_driver.cc"
)

# Compiling ####################################################################
target_precompile_headers(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/comet_rendering_pch.h"
)

target_include_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/"
)

# VMA include compatibility.
find_path(COMET_VMA_INCLUDE_DIR
  NAMES
    vma/vk_mem_alloc.h
    vk_mem_alloc.h
  REQUIRED
)

set(COMET_GENERATED_INCLUDE_DIR
  "${CMAKE_CURRENT_BINARY_DIR}/generated/include"
)

if(EXISTS "${COMET_VMA_INCLUDE_DIR}/vma/vk_mem_alloc.h")
  target_include_directories(${COMET_LIBRARY_NAME}
    PRIVATE
      "${COMET_VMA_INCLUDE_DIR}"
  )
elseif(EXISTS "${COMET_VMA_INCLUDE_DIR}/vk_mem_alloc.h")
  file(MAKE_DIRECTORY "${COMET_GENERATED_INCLUDE_DIR}/vma")

  file(WRITE
    "${COMET_GENERATED_INCLUDE_DIR}/vma/vk_mem_alloc.h"
    "#pragma once\n#include <vk_mem_alloc.h>\n"
  )

  target_include_directories(${COMET_LIBRARY_NAME}
    PRIVATE
      "${COMET_GENERATED_INCLUDE_DIR}"
      "${COMET_VMA_INCLUDE_DIR}"
  )
else()
  message(FATAL_ERROR "Could not find vk_mem_alloc.h")
endif()

if(WIN32)
  target_include_directories(${COMET_LIBRARY_NAME}
    PRIVATE
      "${VULKAN_SDK}/Include"
  )
endif()

if(LINUX)
  target_include_directories(${COMET_LIBRARY_NAME}
    PRIVATE
      "${VULKAN_SDK}/include"
  )
endif()

# Linking ######################################################################
target_link_libraries(${COMET_LIBRARY_NAME}
  PRIVATE
    ZLIB::ZLIB
    glad::glad
    imgui::imgui
    glfw
    Vulkan::Vulkan
)

if(WIN32)
  target_link_libraries(${COMET_LIBRARY_NAME}
    PRIVATE
      bcrypt
      opengl32
  )
endif()

if(LINUX)
  target_link_libraries(${COMET_LIBRARY_NAME}
    PRIVATE
      ${OPENGL_opengl_LIBRARY}
      pthread
      GPUOpen::VulkanMemoryAllocator
      ${CMAKE_DL_LIBS}
  )
endif()
