// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_PRECOMPILE_H_
#define COMET_COMET_PRECOMPILE_H_

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <condition_variable>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include <sys/stat.h>
#include <sys/types.h>

// Order is really important here.
#include "comet/core/compiler.h"
#include "comet/core/os.h"

#ifdef COMET_WINDOWS
#define NOMINMAX
#include <windows.h>

#undef MoveMemory
#undef CopyMemory
#undef FillMemory
#undef ZeroMemory
#endif  // COMET_WINDOWS

#include "comet/core/type/primitive.h"
#include "comet/core/define.h"
#include "comet/core/debug.h"
#include "comet/core/memory/allocation_tracking.h"

#include "comet/core/logger.h"
#include "comet/core/type/gid.h"
#include "comet/core/type/string_id.h"
#include "comet/core/version.h"

#endif  // COMET_COMET_PRECOMPILE_H_
