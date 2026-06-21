// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_RUNTIME_DEFINE_H_
#define COMET_RUNTIME_RUNTIME_DEFINE_H_

#include "comet/build/build_define.h"

// Full Runtime debug checks.
#if defined(COMET_DEBUG) && defined(COMET_FULL_DEBUG_CHECKS)

#define COMET_DEBUG_ENTITY_CHECKS

#endif  // defined(COMET_DEBUG) && defined(COMET_FULL_DEBUG_CHECKS)

// Entities.
#if defined(COMET_DEBUG) && defined(COMET_DEBUG_ENTITY_CHECKS)

#define COMET_DEBUG_ENTITY

#endif  // defined(COMET_DEBUG) && defined(COMET_DEBUG_ENTITY_CHECKS)

// Memory.
#if defined(COMET_DEBUG) && defined(COMET_DEBUG_MEMORY_CHECKS)

#define COMET_DEBUG_STACK_ALLOCATOR
#define COMET_DEBUG_FREE_LIST_ALLOCATOR

#endif  // defined(COMET_DEBUG) && defined(COMET_DEBUG_MEMORY_CHECKS)

#endif  // COMET_RUNTIME_RUNTIME_DEFINE_H_