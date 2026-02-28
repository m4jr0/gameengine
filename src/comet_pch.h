// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_PCH_H_
#define COMET_PCH_H_

// Essentials. /////////////////////////////////////////////////////////////////
#include "comet/core/essentials.h"
////////////////////////////////////////////////////////////////////////////////

#include <float.h>
#include <inttypes.h>
#include <stdio.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwctype>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>

#ifdef COMET_ARCH_X86
#include <emmintrin.h>
#include <xmmintrin.h>
#endif  // COMET_ARCH_X86

#ifdef COMET_MSVC
#include "comet/core/windows.h"
#else
#include <cpuid.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <link.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#endif  // COMET_MSVC

#ifndef COMET_WINDOWS
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif  // !COMET_WINDOWS

#ifdef COMET_TRACK_ALLOCATIONS
#include <new>

#ifdef COMET_MSVC
#include "detours/detours.h"
#endif  // COMET_MSVC
#endif  // COMET_TRACK_ALLOCATIONS

#ifdef COMET_IS_ASAN
#include <sanitizer/asan_interface.h>
#endif  // COMET_IS_ASAN

#ifdef COMET_IS_TSAN
#include <sanitizer/tsan_interface.h>
#endif  // COMET_IS_TSAN

#endif  // COMET_PCH_H_