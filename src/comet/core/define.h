// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_DEFINE_H_
#define COMET_COMET_CORE_DEFINE_H_

#include "comet/core/os.h"

// Misc.
#define COMET_COMMA ,

// String ID.
#ifdef COMET_DEBUG
// TODO(m4jr0): Find lock-free solution.
// #define COMET_LABELIZE_STRING_IDS
#endif  // COMET_DEBUG

// Memory.
#ifdef COMET_DEBUG
// TODO(m4jr0): Find lock-free solution.
// #define COMET_TRACK_ALLOCATIONS
// #define COMET_POISON_ALLOCATIONS
// #define COMET_POISON_FIBER_STACKS

#define COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
#endif  // COMET_DEBUG

// Thread.
#define COMET_RESERVE_SYSTEM_THREADS

// Fiber.
#ifdef COMET_DEBUG
#define COMET_FIBER_DEBUG_LABEL
#endif  // COMET_DEBUG

// Jobs.
#ifdef COMET_DEBUG
#define COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
#endif  // COMET_DEBUG

// Logging.
#ifdef COMET_DEBUG
#define COMET_LOG_IS_FIBER_PREFIX
#endif  // COMET_DEBUG

// String.
#ifdef COMET_WINDOWS
#define COMET_WIDE_TCHAR
#endif  // COMET_WINDOWS

// Path.
#define COMET_NORMALIZE_PATHS

// Animation.
#define COMET_COMPRESS_ANIMATIONS

// Math.
// #define COMET_USE_LERP_FOR_SLERP

// Rendering.
#ifdef COMET_DEBUG
#define COMET_PROFILING
#define COMET_IMGUI

// #define COMET_DEBUG_RENDERING
#ifdef COMET_DEBUG_RENDERING
// Assign a name to driver's objects (when available).
#define COMET_RENDERING_USE_DEBUG_LABELS

// RenderDoc might (will?) crash if this is not enabled.
#define COMET_ENABLE_RENDERDOC_COMPATIBILITY

// View used for debugging the world.
// #define COMET_DEBUG_VIEW

// Print debug messages from VMA.
// #define COMET_VULKAN_DEBUG_VMA

// Compile shaders with debug info and no optimizations.
#define COMET_DEBUG_SHADER

// Display some debug data about culling.
// #define COMET_DEBUG_CULLING

// According to the Vulkan spec, it is better to enable the validation layers
// individually to prevent a significant performance degradation.
// #define COMET_VALIDATION_GPU_ASSISTED_EXT
// #define COMET_VALIDATION_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT
// #define COMET_VALIDATION_BEST_PRACTICES_EXT
#define COMET_VALIDATION_DEBUG_PRINTF_EXT
#define COMET_VALIDATION_SYNCHRONIZATION_VALIDATION_EXT
#endif  // COMET_DEBUG_RENDERING
#endif  // COMET_DEBUG

#endif  // COMET_COMET_CORE_DEFINE_H_
