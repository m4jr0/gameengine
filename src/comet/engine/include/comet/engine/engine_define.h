// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_ENGINE_ENGINE_DEFINE_H_
#define COMET_ENGINE_ENGINE_DEFINE_H_

#include "comet/build/build_define.h"

// Debug UI.
#if defined(COMET_DEBUG) && defined(COMET_IMGUI)

#define COMET_HAS_DEBUG_UI

#endif  // defined(COMET_DEBUG) && defined(COMET_IMGUI)

// Profiler debug UI.
#if defined(COMET_HAS_DEBUG_UI) && defined(COMET_PROFILING)

#define COMET_HAS_PROFILER_DEBUG_UI

#endif  // defined(COMET_HAS_DEBUG_UI) && defined(COMET_PROFILING)

// Memory debug UI.
#if defined(COMET_HAS_PROFILER_DEBUG_UI) && \
    defined(COMET_DEBUG_TRACK_ALLOCATIONS)

#define COMET_HAS_MEMORY_DEBUG_UI

#endif  // defined(COMET_HAS_PROFILER_DEBUG_UI) &&
        // defined(COMET_DEBUG_TRACK_ALLOCATIONS)

#endif  // COMET_ENGINE_ENGINE_DEFINE_H_