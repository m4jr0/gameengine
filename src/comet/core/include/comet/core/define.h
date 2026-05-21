// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_DEFINE_H_
#define COMET_CORE_DEFINE_H_
// >:3 Split this thing...

#include "comet/core/os.h"

// Misc.
#define COMET_COMMA ,

#define COMET_CONCAT_IMPL(x, y) x##y
#define COMET_CONCAT(x, y) COMET_CONCAT_IMPL(x, y)

// Full debug checks.
// Enables every expensive, noisy, or intrusive debug feature used for pre-merge
// validation builds.
//
// Prefer enabling it from CMake:
//   cmake -B build -DCOMET_FULL_DEBUG_CHECKS=ON
//
// This only affects debug builds.
#if defined(COMET_DEBUG) && defined(COMET_FULL_DEBUG_CHECKS)

#define COMET_DEBUG_LABELIZE_STRING_IDS
#define COMET_DEBUG_MEMORY_CHECKS
#define COMET_DEBUG_TRACK_ALLOCATIONS
#define COMET_DEBUG_LOG_CONTEXT_PREFIX
#define COMET_DEBUG_ENTITY_CHECKS
#define COMET_DEBUG_RENDERING_CHECKS

#endif  // defined(COMET_DEBUG) && defined(COMET_FULL_DEBUG_CHECKS)

#if defined(COMET_DEBUG) && defined(COMET_DEBUG_LABELIZE_STRING_IDS)

#define COMET_LABELIZE_STRING_IDS

#endif  // defined(COMET_DEBUG) && defined(COMET_DEBUG_LABELIZE_STRING_IDS)

// Allocation tracking.
// Tracks allocations globally. This can be useful on its own when looking for
// leaks or ownership issues without enabling every memory debug feature.
#if defined(COMET_DEBUG) && defined(COMET_DEBUG_TRACK_ALLOCATIONS)

#define COMET_TRACK_ALLOCATIONS

#endif  // defined(COMET_DEBUG) && defined(COMET_DEBUG_TRACK_ALLOCATIONS)

// Memory debug checks.
// Enables expensive memory diagnostics such as poisoning, allocator checks,
// verbose allocator logs, and allocation tracking.
#if defined(COMET_DEBUG) && defined(COMET_DEBUG_MEMORY_CHECKS)

#define COMET_POISON_ALLOCATIONS
#define COMET_POISON_FIBER_STACKS
#define COMET_DEBUG_STACK_ALLOCATOR
#define COMET_DEBUG_FREE_LIST_ALLOCATOR
#define COMET_DEBUG_STRING_ID_ALLOCATOR
#define COMET_VERBOSE_ALLOCATOR_LOGS

#endif  // defined(COMET_DEBUG) && defined(COMET_DEBUG_MEMORY_CHECKS)

// Logging context prefix.
// Displays the forwarded log prefix, for example Class::Method.
#if defined(COMET_DEBUG) && defined(COMET_DEBUG_LOG_CONTEXT_PREFIX)

#define COMET_LOG_USE_CONTEXT_PREFIX

#endif  // defined(COMET_DEBUG) && defined(COMET_DEBUG_LOG_CONTEXT_PREFIX)

// Entity debug checks.
// Enables extra entity diagnostics.
#if defined(COMET_DEBUG) && defined(COMET_DEBUG_ENTITY_CHECKS)

#define COMET_DEBUG_ENTITY

#endif  // defined(COMET_DEBUG) && defined(COMET_DEBUG_ENTITY_CHECKS)

// Rendering debug checks.
// Enables rendering diagnostics and debug views.
#if defined(COMET_DEBUG) && defined(COMET_DEBUG_RENDERING_CHECKS)

#define COMET_DEBUG_RENDERING
#define COMET_RENDERING_USE_DEBUG_LABELS
#define COMET_DEBUG_VIEW

#endif  // defined(COMET_DEBUG) && defined(COMET_DEBUG_RENDERING_CHECKS)

// Jobs.
// #define COMET_ALLOW_WORKER_SLEEP
// OpenGL won't run without it...
#define COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER

// Memory.
#ifdef COMET_DEBUG
#define COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
#endif  // COMET_DEBUG

// Thread.
#define COMET_RESERVE_SYSTEM_THREADS

// Fiber.
#ifdef COMET_DEBUG
#define COMET_LOG_USE_FIBER_PREFIX
#define COMET_FIBER_DEBUG_LABEL
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
#define COMET_RENDERING_OPENGL_CLIP_CONTROL_ZERO_TO_ONE

#ifdef COMET_DEBUG
#define COMET_PROFILING
#define COMET_IMGUI

#ifdef COMET_DEBUG_RENDERING
// RenderDoc might (will?) crash if this is not enabled.
#define COMET_ENABLE_RENDERDOC_COMPATIBILITY

// Print debug messages from VMA.
// #define COMET_VULKAN_DEBUG_VMA

// Compile shaders with debug info and no optimizations.
#define COMET_DEBUG_SHADER

// According to the Vulkan spec, it is better to enable the validation layers
// individually to prevent a significant performance degradation.
// #define COMET_VALIDATION_GPU_ASSISTED_EXT
// #define COMET_VALIDATION_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT
// #define COMET_VALIDATION_BEST_PRACTICES_EXT
#define COMET_VALIDATION_DEBUG_PRINTF_EXT
#define COMET_VALIDATION_SYNCHRONIZATION_VALIDATION_EXT
#endif  // COMET_DEBUG_RENDERING
#endif  // COMET_DEBUG

// Debug UIs.

#if defined(COMET_DEBUG) && defined(COMET_IMGUI)
#define COMET_HAS_DEBUG_UI
#endif  // defined(COMET_DEBUG) && defined(COMET_IMGUI)

#if defined(COMET_HAS_DEBUG_UI) && defined(COMET_PROFILING)
#define COMET_HAS_PROFILER_DEBUG_UI
#endif  // defined(COMET_HAS_DEBUG_UI) && defined(COMET_PROFILING)

#if defined(COMET_HAS_PROFILER_DEBUG_UI) && defined(COMET_TRACK_ALLOCATIONS)
#define COMET_HAS_MEMORY_DEBUG_UI
#endif  // defined(COMET_HAS_PROFILER_DEBUG_UI) &&
        // defined(COMET_TRACK_ALLOCATIONS)

#endif  // COMET_CORE_DEFINE_H_
