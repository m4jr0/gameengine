// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_PLATFORM_SRC_COMET_PLATFORM_PCH_H_
#define COMET_PLATFORM_SRC_COMET_PLATFORM_PCH_H_

// Essentials. /////////////////////////////////////////////////////////////////
#include "comet/core/essentials.h"
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>

#include "comet/core/essentials.h"

#ifdef COMET_MSVC
#include "comet/core/windows.h"
#else
#include <signal.h>
#endif  // COMET_MSVC

#endif  // COMET_PLATFORM_SRC_COMET_PLATFORM_PCH_H_