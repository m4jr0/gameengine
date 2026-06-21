# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet render library CMake file
#
################################################################################

add_library(${COMET_RENDER_TARGET} STATIC)

comet_configure_target(${COMET_RENDER_TARGET})

# Packages #####################################################################
find_package(glad CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

if(NOT WIN32)
  find_package(VulkanMemoryAllocator CONFIG REQUIRED)
endif()

# Source files #################################################################
set(COMET_RENDER_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/render_define.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/debug/rendering_debug_settings.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/debug_ui_registry.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/driver.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/empty/empty_driver.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/handler/opengl_camera_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/handler/opengl_debug_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/handler/opengl_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/handler/opengl_lighting_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/handler/opengl_material_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/handler/opengl_mesh_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/handler/opengl_render_proxy_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/handler/opengl_sampler_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/handler/opengl_shader_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/handler/opengl_shader_module_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/handler/opengl_texture_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/handler/opengl_view_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/label/opengl_texture_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/label/opengl_view_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/opengl_debug.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/opengl_driver.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/opengl_frame_state.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_buffer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_camera.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_debug.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_descriptor.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_frame.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_framebuffer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_light.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_material.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_mesh.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_pipeline.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_region_gpu_buffer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_render_proxy.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_sampler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_shader.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_shader_module.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_shadow.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_storage.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_texture.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_texture_map.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/type/opengl_view.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/utils/opengl_sampler_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/utils/opengl_shader_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/utils/opengl_texture_map_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/utils/opengl_texture_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/utils/opengl_view_shader_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/view/opengl_debug_view.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/view/opengl_imgui_view.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/view/opengl_shadow_view.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/view/opengl_view.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/opengl/view/opengl_world_view.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_camera_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_debug_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_descriptor_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_lighting_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_material_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_mesh_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_pipeline_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_render_pass_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_render_proxy_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_sampler_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_shader_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_shader_module_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_texture_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/handler/vulkan_view_handler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/label/vulkan_descriptor_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/label/vulkan_render_pass_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/label/vulkan_texture_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/label/vulkan_view_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_buffer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_camera.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_command_buffer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_debug.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_descriptor.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_frame.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_image.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_light.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_material.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_mesh.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_pipeline.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_region_gpu_buffer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_render_pass.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_render_proxy.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_sampler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_shader.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_shader_module.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_shadow.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_texture.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_texture_map.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/type/vulkan_view.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/utils/vulkan_buffer_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/utils/vulkan_command_buffer_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/utils/vulkan_descriptor_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/utils/vulkan_image_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/utils/vulkan_initializer_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/utils/vulkan_pipeline_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/utils/vulkan_render_pass_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/utils/vulkan_sampler_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/utils/vulkan_shader_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/utils/vulkan_texture_map_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/utils/vulkan_texture_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/utils/vulkan_view_shader_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/utils/vulkan_view_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/view/vulkan_debug_view.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/view/vulkan_imgui_view.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/view/vulkan_shadow_view.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/view/vulkan_view.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/view/vulkan_world_view.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/vulkan_alloc.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/vulkan_context.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/vulkan_debug.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/vulkan_device.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/vulkan_driver.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/driver/vulkan/vulkan_swapchain.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/common.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/frustum.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/label/common_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/label/view_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/render_proxy_record_store.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/render_handle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/render_manager.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/render_proxy.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/view.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/utils/culling_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/utils/driver_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/render/utils/geometry_utils.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/debug/rendering_debug_settings.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/debug_ui_registry.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/driver.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/empty/empty_driver.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/handler/opengl_camera_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/handler/opengl_debug_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/handler/opengl_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/handler/opengl_lighting_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/handler/opengl_material_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/handler/opengl_mesh_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/handler/opengl_render_proxy_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/handler/opengl_sampler_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/handler/opengl_shader_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/handler/opengl_shader_module_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/handler/opengl_texture_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/handler/opengl_view_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/label/opengl_texture_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/label/opengl_view_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/opengl_debug.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/opengl_driver.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/opengl_frame_state.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/utils/opengl_sampler_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/utils/opengl_shader_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/utils/opengl_texture_map_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/utils/opengl_texture_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/utils/opengl_view_shader_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/view/opengl_debug_view.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/view/opengl_imgui_view.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/view/opengl_shadow_view.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/view/opengl_view.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/opengl/view/opengl_world_view.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_camera_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_debug_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_descriptor_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_lighting_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_material_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_mesh_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_pipeline_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_render_pass_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_render_proxy_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_sampler_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_shader_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_shader_module_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_texture_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/handler/vulkan_view_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/label/vulkan_descriptor_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/label/vulkan_render_pass_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/label/vulkan_texture_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/label/vulkan_view_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/utils/vulkan_buffer_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/utils/vulkan_command_buffer_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/utils/vulkan_descriptor_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/utils/vulkan_image_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/utils/vulkan_initializer_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/utils/vulkan_pipeline_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/utils/vulkan_render_pass_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/utils/vulkan_sampler_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/utils/vulkan_shader_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/utils/vulkan_texture_map_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/utils/vulkan_texture_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/utils/vulkan_view_shader_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/utils/vulkan_view_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/view/vulkan_debug_view.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/view/vulkan_imgui_view.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/view/vulkan_shadow_view.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/view/vulkan_view.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/view/vulkan_world_view.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/vulkan_alloc.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/vulkan_context.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/vulkan_debug.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/vulkan_device.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/vulkan_driver.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/vulkan_swapchain.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/driver/vulkan/vulkan_vma.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/frustum.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/label/common_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/label/view_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/render_proxy_record_store.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/render_manager.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/culling_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/driver_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/geometry_utils.cc"
)

target_sources(${COMET_RENDER_TARGET}
  PRIVATE
    ${COMET_RENDER_SOURCES}
)

# Compiling ####################################################################
target_precompile_headers(${COMET_RENDER_TARGET}
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/comet_render_pch.h"
)

target_include_directories(${COMET_RENDER_TARGET}
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
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
  target_include_directories(${COMET_RENDER_TARGET}
    PRIVATE
      "${COMET_VMA_INCLUDE_DIR}"
  )
elseif(EXISTS "${COMET_VMA_INCLUDE_DIR}/vk_mem_alloc.h")
  file(MAKE_DIRECTORY "${COMET_GENERATED_INCLUDE_DIR}/vma")

  file(WRITE
    "${COMET_GENERATED_INCLUDE_DIR}/vma/vk_mem_alloc.h"
    "#pragma once\n#include <vk_mem_alloc.h>\n"
  )

  target_include_directories(${COMET_RENDER_TARGET}
    PRIVATE
      "${COMET_GENERATED_INCLUDE_DIR}"
      "${COMET_VMA_INCLUDE_DIR}"
  )
else()
  message(FATAL_ERROR "Could not find vk_mem_alloc.h")
endif()

if(WIN32)
  target_include_directories(${COMET_RENDER_TARGET}
    PRIVATE
      "${VULKAN_SDK}/Include"
  )
endif()

if(LINUX)
  target_include_directories(${COMET_RENDER_TARGET}
    PRIVATE
      "${VULKAN_SDK}/include"
  )
endif()

# Precompiled headers ##########################################################
target_precompile_headers(${COMET_RENDER_TARGET}
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/comet_render_pch.h"
)

# Linking ######################################################################
target_link_libraries(${COMET_RENDER_TARGET}
  PUBLIC
    ${COMET_CORE_TARGET}
    ${COMET_DATA_TARGET}
    ${COMET_RUNTIME_TARGET}
    ${COMET_PLATFORM_TARGET}
    glad::glad
    Vulkan::Vulkan
    imgui::imgui
  PRIVATE
    ZLIB::ZLIB
)

if(WIN32)
  target_link_libraries(${COMET_RENDER_TARGET}
    PRIVATE
      bcrypt
      opengl32
  )
endif()

if(LINUX)
  target_link_libraries(${COMET_RENDER_TARGET}
    PRIVATE
      ${OPENGL_opengl_LIBRARY}
      pthread
      GPUOpen::VulkanMemoryAllocator
      ${CMAKE_DL_LIBS}
  )
endif()