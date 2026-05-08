# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet debugging library CMake file
#
################################################################################

# Packages #####################################################################
# ImGui.
find_package(imgui CONFIG REQUIRED)

# Source files #################################################################
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet/debugging/ui/debug_ui_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/debugging/ui/imgui_utils.cc"

    # Rendering.
    "${PROJECT_SOURCE_DIR}/src/comet/debugging/rendering/rendering_debug_settings.cc"

    # UI.
    # Profiler.
    "${PROJECT_SOURCE_DIR}/src/comet/debugging/ui/debugger/debugger_debug_ui.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/debugging/ui/debugger/memory/allocation_tracker_displayer.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/debugging/ui/debugger/profiler/cpu_profiler_displayer.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/debugging/ui/debugger/profiler/cpu_profiler_displayer_context.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/debugging/ui/debugger/profiler/cpu_profiler_graph.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/debugging/ui/debugger/profiler/cpu_profiler_tree.cc"

    # Rendering.
    "${PROJECT_SOURCE_DIR}/src/comet/debugging/ui/rendering/rendering_debug_ui.cc"

    # Environment.
    "${PROJECT_SOURCE_DIR}/src/comet/debugging/ui/environment/environment_debug_ui.cc"
)

# Compiling ####################################################################
target_include_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/"
)