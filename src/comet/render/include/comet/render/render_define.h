// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_RENDER_DEFINE_H_
#define COMET_RENDER_RENDER_DEFINE_H_

#include "comet/build/build_define.h"

// Full rendering debug checks.
#if defined(COMET_DEBUG) && defined(COMET_FULL_DEBUG_CHECKS)

#define COMET_DEBUG_RENDERING_CHECKS

#endif  // defined(COMET_DEBUG) && defined(COMET_FULL_DEBUG_CHECKS)

// OpenGL.
#define COMET_RENDERING_OPENGL_CLIP_CONTROL_ZERO_TO_ONE

// Rendering diagnostics.
#if defined(COMET_DEBUG) && defined(COMET_DEBUG_RENDERING_CHECKS)

#define COMET_DEBUG_RENDERING
#define COMET_RENDERING_USE_DEBUG_LABELS
#define COMET_DEBUG_VIEW

// RenderDoc might crash if this compatibility mode is not enabled.
#define COMET_ENABLE_RENDERDOC_COMPATIBILITY

// Compile shaders with debug information and disabled optimizations.
#define COMET_DEBUG_SHADER

// Print debug messages from VMA.
// #define COMET_VULKAN_DEBUG_VMA

// According to the Vulkan specification, it is preferable to enable
// validation features individually to avoid unnecessary overhead.
// #define COMET_VALIDATION_GPU_ASSISTED_EXT
// #define COMET_VALIDATION_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT
// #define COMET_VALIDATION_BEST_PRACTICES_EXT
#define COMET_VALIDATION_DEBUG_PRINTF_EXT
#define COMET_VALIDATION_SYNCHRONIZATION_VALIDATION_EXT

#endif  // defined(COMET_DEBUG) && defined(COMET_DEBUG_RENDERING_CHECKS)

#endif  // COMET_RENDER_RENDER_DEFINE_H_