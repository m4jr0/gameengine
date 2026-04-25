# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet debug UI library CMake file
#
################################################################################

# Packages #####################################################################
# ImGui.
find_package(imgui CONFIG REQUIRED)

# Source files #################################################################
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet/debug_ui/debug_ui_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/debug_ui/imgui_utils.cc"

    # Profiler.
    "${PROJECT_SOURCE_DIR}/src/comet/debug_ui/debugger/debugger_debug_ui.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/debug_ui/debugger/memory/allocation_tracker_displayer.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/debug_ui/debugger/profiler/cpu_profiler_displayer.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/debug_ui/debugger/profiler/cpu_profiler_displayer_context.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/debug_ui/debugger/profiler/cpu_profiler_graph.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/debug_ui/debugger/profiler/cpu_profiler_tree.cc"

    # Environment.
    "${PROJECT_SOURCE_DIR}/src/comet/debug_ui/environment/environment_debug_ui.cc"
)

# Compiling ####################################################################
target_include_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/"
)