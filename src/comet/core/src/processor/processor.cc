// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be it in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/processor/processor.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#ifdef COMET_MSVC
#include <intrin.h>
#else
#include <cpuid.h>
#endif  // COMET_MSVC
////////////////////////////////////////////////////////////////////////////////

namespace comet {
bool IsAVXSupported() {
#ifdef COMET_MSVC
  int cpu_info[4];
  __cpuid(cpu_info, 1);
  return (cpu_info[2] & (1 << 28)) != 0;
#else
  unsigned int eax;
  unsigned int ebx;
  unsigned int ecx;
  unsigned int edx;

  if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
    return (ecx & bit_AVX) != 0;
  }

  return false;
#endif  // COMET_MSVC
}
}  // namespace comet
