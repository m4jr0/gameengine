# Copyright 2025 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet rendering library CMake file
#
################################################################################

# Packages #####################################################################
# GLFW.
find_package(glfw3 REQUIRED)

# OpenGL.
find_package(glad REQUIRED)
find_package(OpenGL REQUIRED)

# Vulkan.
find_package(Vulkan REQUIRED)

if(NOT WIN32)
  find_package(VulkanMemoryAllocator CONFIG REQUIRED)
endif()

# ImGui.
find_package(imgui CONFIG REQUIRED)

if(LINUX)
  # X11.
  find_package(X11 REQUIRED)
endif()

if(WIN32 AND NOT DEFINED VULKAN_SDK)
  if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR
      "${MESSAGE_PREFIX}VULKAN_SDK is not set. "
      "Please define VULKAN_SDK to your Vulkan SDK root directory. "
      "Aborting.")
  endif()

  set(VULKAN_SDK $ENV{VULKAN_SDK})
endif()

# Source files #################################################################
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/rendering_common.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/rendering_manager.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/rendering_utils.cc"

      # Camera.
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/camera/camera.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/camera/camera_manager.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/camera/frustum.cc"

      # Debugger.
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/debugger/debugger_displayer_manager.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/debugger/imgui_utils.cc"
      
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/debugger/memory/allocation_tracker_displayer.cc"

      "${PROJECT_SOURCE_DIR}/src/comet/rendering/debugger/profiler/cpu_profiler_displayer.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/debugger/profiler/cpu_profiler_displayer_context.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/debugger/profiler/cpu_profiler_graph.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/debugger/profiler/cpu_profiler_tree.cc"

      # Driver.
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/driver.cc"

      # OpenGL.
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/opengl_driver.cc"
    
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/data/opengl_frame.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/data/opengl_material.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/data/opengl_mesh.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/data/opengl_render_proxy.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/data/opengl_shader.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/data/opengl_shader_module.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/data/opengl_texture.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/data/opengl_texture_map.h"

      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_material_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_mesh_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_render_proxy_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_shader_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_shader_module_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_texture_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/handler/opengl_view_handler.cc"

      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/view/opengl_debug_view.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/view/opengl_imgui_view.cc"  
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/view/opengl_shader_view.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/view/opengl_view.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/view/opengl_world_view.cc"

    # Vulkan.
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_buffer.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_command_buffer.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_descriptor.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_frame.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_image.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_material.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_mesh.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_pipeline.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_render_pass.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_render_proxy.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_shader.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_shader_data.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_shader_module.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_texture.h"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/data/vulkan_texture_map.h"
    
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_descriptor_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_material_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_mesh_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_pipeline_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_render_pass_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_render_proxy_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_shader_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_shader_module_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_texture_handler.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/handler/vulkan_view_handler.cc"

      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_common_utils.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_descriptor_utils.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_image_utils.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/utils/vulkan_material_utils.cc"

      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/view/vulkan_debug_view.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/view/vulkan_imgui_view.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/view/vulkan_shader_view.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/view/vulkan_view.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/view/vulkan_world_view.cc"

      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_alloc.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_context.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_debug.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_device.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_driver.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_swapchain.cc"

    # Empty.
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/empty/empty_driver.cc"

      # Windows.
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/glfw/glfw_window.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/glfw/empty/empty_glfw_window.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/glfw/opengl/opengl_glfw_window.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/glfw/vulkan/vulkan_glfw_window.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/window.cc"
      "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/window_event.cc"
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
)

if(WIN32)
  target_link_libraries(${COMET_LIBRARY_NAME}
    PRIVATE
      "${VULKAN_SDK}/Lib/vulkan-1.lib"
      bcrypt
      opengl32
      glfw3dll
  )
endif()

if(LINUX)
  target_link_libraries(${COMET_LIBRARY_NAME}
    PRIVATE
      ${OPENGL_opengl_LIBRARY}
      glfw3
      pthread
      ${Vulkan_LIBRARIES}
      GPUOpen::VulkanMemoryAllocator
      ${CMAKE_DL_LIBS}
      ${X11_X11_LIB}
  )
endif()
