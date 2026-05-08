// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/logger/logging.h"

#ifdef COMET_VULKAN_DEBUG_VMA

#define VMA_DEBUG_LOG(format, ...)                                            \
  do {                                                                        \
    constexpr auto kMessageLength{512};                                       \
    char message[kMessageLength]{'\0'};                                       \
    std::snprintf(message, kMessageLength - 1, format, ##__VA_ARGS__);        \
    COMET_LOG_DEBUG(comet::LoggerType::Rendering, "vma", "vma debug message", \
                    "message", message);                                      \
  } while (false)

#endif  // COMET_VULKAN_DEBUG_VMA

#define VMA_ASSERT(cond) COMET_ASSERT((cond), "vma", "vma assert failed")

// External. ///////////////////////////////////////////////////////////////////
#define VMA_IMPLEMENTATION

#ifdef COMET_MSVC
#include "vma/vk_mem_alloc.h"
#else
#include "vk_mem_alloc.h"
#endif  // COMET_MSVC
////////////////////////////////////////////////////////////////////////////////