// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_DEFINE_H_
#define COMET_COMET_CORE_DEFINE_H_

#include "comet/core/os.h"

// String.
#ifdef COMET_WINDOWS
#define COMET_WIDE_TCHAR
#endif  // COMET_WINDOWS

// Path.
#define COMET_NORMALIZE_PATHS

// Rendering.
#ifdef COMET_DEBUG
#define COMET_PROFILING
#define COMET_IMGUI

#define COMET_RENDERING_DRIVER_DEBUG_MODE
#ifdef COMET_RENDERING_DRIVER_DEBUG_MODE
// View used for debugging the world.
// #define COMET_DEBUG_VIEW

// Print debug messages from VMA.
// #define COMET_VULKAN_DEBUG_VMA

// According to the Vulkan spec, it is better to enable the validation layers
// individually to prevent a significant performance degradation.
// #define COMET_VALIDATION_GPU_ASSISTED_EXT
// #define COMET_VALIDATION_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT
// #define COMET_VALIDATION_BEST_PRACTICES_EXT
// #define COMET_VALIDATION_DEBUG_PRINTF_EXT
// #define COMET_VALIDATION_SYNCHRONIZATION_VALIDATION_EXT
#endif  // COMET_RENDERING_DRIVER_DEBUG_MODE
#endif  // COMET_DEBUG

#endif  // COMET_COMET_CORE_DEFINE_H_
