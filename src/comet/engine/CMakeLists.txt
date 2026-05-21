# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet engine library CMake file
#
################################################################################

add_library(${COMET_ENGINE_TARGET} STATIC)

comet_configure_target(${COMET_ENGINE_TARGET})

# Packages #####################################################################
find_package(imgui CONFIG REQUIRED)

# Source files #################################################################
set(COMET_ENGINE_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/debug/ui/debug_ui_manager.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/debug/ui/debugger_debug_ui.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/debug/ui/environment/environment_debug_ui.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/debug/ui/imgui_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/debug/ui/memory/allocation_tracker_displayer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/debug/ui/profiler/cpu_profiler_displayer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/debug/ui/profiler/cpu_profiler_displayer_context.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/debug/ui/profiler/cpu_profiler_graph.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/debug/ui/profiler/cpu_profiler_tree.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/debug/ui/rendering/rendering_debug_ui.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/engine.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/engine_client.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/engine_event.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/profiler/profiler_data.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/profiler/profiler_data_collector.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/engine/threading/main_thread_worker_policy.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/debug/ui/debug_ui_manager.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/debug/ui/debugger_debug_ui.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/debug/ui/environment/environment_debug_ui.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/debug/ui/imgui_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/debug/ui/memory/allocation_tracker_displayer.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/debug/ui/profiler/cpu_profiler_displayer.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/debug/ui/profiler/cpu_profiler_displayer_context.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/debug/ui/profiler/cpu_profiler_graph.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/debug/ui/profiler/cpu_profiler_tree.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/debug/ui/rendering/rendering_debug_ui.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/engine.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/engine_event.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/profiler/profiler_data.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/profiler/profiler_data_collector.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/threading/main_thread_worker_policy.cc"
)

target_sources(${COMET_ENGINE_TARGET}
  PRIVATE
    ${COMET_ENGINE_SOURCES}
)

# Compiling ####################################################################
target_include_directories(${COMET_ENGINE_TARGET}
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Linking ######################################################################
target_link_libraries(${COMET_ENGINE_TARGET}
  PUBLIC
    ${COMET_CORE_TARGET}
    ${COMET_DATA_TARGET}
    ${COMET_RUNTIME_TARGET}
    ${COMET_PLATFORM_TARGET}
    ${COMET_RENDER_TARGET}
    imgui::imgui
)