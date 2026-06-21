// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_SRC_COMET_RENDER_PCH_H_
#define COMET_RENDER_SRC_COMET_RENDER_PCH_H_

// Essentials. /////////////////////////////////////////////////////////////////
#include "comet/core/essentials.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/render/render_define.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef COMET_MSVC
#include "comet/core/windows.h"
#endif  // COMET_MSVC

#include "glad/glad.h"
#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#endif  // COMET_RENDER_SRC_COMET_RENDER_PCH_H_