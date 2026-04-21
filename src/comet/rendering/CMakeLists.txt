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
find_package(glad REQUIRED)
find_package(OpenGL REQUIRED)

# Vulkan.
if(LINUX)
  # Using the VCPKG's loader create issues with Wayland...
  set(Vulkan_LIBRARY "/lib/x86_64-linux-gnu/libvulkan.so.1" CACHE FILEPATH "" FORCE)
  set(Vulkan_INCLUDE_DIR "/usr/include" CACHE PATH "" FORCE)
endif()

find_package(Vulkan REQUIRED)

if(NOT WIN32)
  find_package(VulkanMemoryAllocator CONFIG REQUIRED)
endif()

# ImGui.
find_package(imgui CONFIG REQUIRED)

# Workaround: Some vcpkg installations place "vk_mem_alloc.h" directly in the 
# include path instead of under "vma/". This ensures consistent include paths
# across platforms.
if(NOT EXISTS "${VCPKG_INCLUDE_DIR}/vma/vk_mem_alloc.h")
  if(EXISTS "${VCPKG_INCLUDE_DIR}/vk_mem_alloc.h")
    file(MAKE_DIRECTORY "${VCPKG_INCLUDE_DIR}/vma")

    file(CREATE_LINK
      "${VCPKG_INCLUDE_DIR}/vk_mem_alloc.h"
      "${VCPKG_INCLUDE_DIR}/vma/vk_mem_alloc.h"
      SYMBOLIC)
  endif()
endif()

# Source files #################################################################
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/camera.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/camera_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/debug_ui_registry.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/frustum.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/light_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/rendering_handle.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/rendering_manager.cc"

    # Label.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/label/rendering_common_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/label/rendering_light_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/label/rendering_pipeline_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/label/rendering_shader_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/label/rendering_texture_label.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/label/rendering_view_label.cc"

    # Type.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/rendering_camera_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/rendering_common_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/rendering_light_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/rendering_pipeline_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/rendering_shader_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/rendering_texture_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/type/rendering_view_type.h"

    # Utils.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/rendering_camera_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/rendering_culling_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/rendering_driver_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/rendering_geometry_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/rendering_light_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/rendering_shader_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/utils/rendering_texture_utils.cc"

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
    
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_frame_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_framebuffer_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_light_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_material_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_mesh_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_pipeline_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_region_gpu_buffer_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_render_proxy_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_sampler_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_shader_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_shader_module_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_shadow_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_storage_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_texture_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/type/opengl_texture_map_type.h"
    
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

    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_buffer_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_command_buffer_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_descriptor_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_frame_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_image_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_light_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_material_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_mesh_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_pipeline_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_region_gpu_buffer_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_render_pass_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_render_proxy_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_sampler_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_shader_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_shader_module_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_texture_type.h"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/type/vulkan_texture_map_type.h"

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
  PRIVATE SYSTEM
    ${VCPKG_INCLUDE_DIR}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/"
)

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
target_link_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${COMET_LIBRARY_NAME}
  PRIVATE
    ${ZLIB_LIBRARY_RELEASE}
    glad
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
