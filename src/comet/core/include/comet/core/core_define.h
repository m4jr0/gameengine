// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_CORE_DEFINE_H_
#define COMET_CORE_CORE_DEFINE_H_

#include "comet/build/build_define.h"

// Preprocessor utilities.
#define COMET_COMMA ,

#define COMET_CONCAT_IMPL(x, y) x##y
#define COMET_CONCAT(x, y) COMET_CONCAT_IMPL(x, y)

// Full Core debug checks.
#if defined(COMET_DEBUG) && defined(COMET_FULL_DEBUG_CHECKS)

#define COMET_DEBUG_LABELIZE_STRING_IDS
#define COMET_DEBUG_LOG_CONTEXT_PREFIX

#endif  // defined(COMET_DEBUG) && defined(COMET_FULL_DEBUG_CHECKS)

// String IDs.
#if defined(COMET_DEBUG) && defined(COMET_DEBUG_LABELIZE_STRING_IDS)

#define COMET_LABELIZE_STRING_IDS

#endif  // defined(COMET_DEBUG) && defined(COMET_DEBUG_LABELIZE_STRING_IDS)

// Memory.
#if defined(COMET_DEBUG) && defined(COMET_DEBUG_MEMORY_CHECKS)

#define COMET_POISON_ALLOCATIONS
#define COMET_POISON_FIBER_STACKS
#define COMET_DEBUG_STRING_ID_ALLOCATOR

#endif  // defined(COMET_DEBUG) && defined(COMET_DEBUG_MEMORY_CHECKS)

#ifdef COMET_DEBUG

#define COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS

#endif  // COMET_DEBUG

// Logging.
#if defined(COMET_DEBUG) && defined(COMET_DEBUG_LOG_CONTEXT_PREFIX)

#define COMET_LOG_USE_CONTEXT_PREFIX

#endif  // defined(COMET_DEBUG) && defined(COMET_DEBUG_LOG_CONTEXT_PREFIX)

// Jobs.
// #define COMET_ALLOW_WORKER_SLEEP

// Threads.
#define COMET_RESERVE_SYSTEM_THREADS

// Fibers.
#ifdef COMET_DEBUG

#define COMET_LOG_USE_FIBER_PREFIX
#define COMET_FIBER_DEBUG_LABEL

#endif  // COMET_DEBUG

// Strings.
#ifdef COMET_WINDOWS

#define COMET_WIDE_TCHAR

#endif  // COMET_WINDOWS

// Paths.
#define COMET_NORMALIZE_PATHS

#endif  // COMET_CORE_CORE_DEFINE_H_