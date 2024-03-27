// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_JOB_JOB_H_
#define COMET_COMET_CORE_JOB_JOB_H_

#include "comet_precompile.h"

namespace comet {
namespace job {
using ParamsHandle = uptr;
constexpr auto kInvalidParamsHandle{static_cast<ParamsHandle>(-1)};

using EntryPoint = std::function<void(ParamsHandle params)>;

enum class JobPriority { Unknown = 0, Low, Normal, High, Critical };

struct Counter {};

struct JobDescr {
  EntryPoint entry_point{};
  ParamsHandle params_handle{kInvalidParamsHandle};
  JobPriority priority{JobPriority::Unknown};
  Counter* counter{nullptr};
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_JOB_H_