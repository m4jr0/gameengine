// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_SANDBOX_PCH_H_
#define COMET_SANDBOX_PCH_H_

// Essentials. /////////////////////////////////////////////////////////////////
#include "comet/core/essentials.h"
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "comet/core/essentials.h"

#ifdef COMET_MSVC
#include "comet/core/windows.h"
#else
#include <signal.h>
#endif  // COMET_MSVC

#endif  // COMET_SANDBOX_PCH_H_