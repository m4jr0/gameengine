// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_PROFILER_PROFILER_H_
#define COMET_CORE_PROFILER_PROFILER_H_

#include "comet/core/essentials.h"

#ifdef COMET_PROFILING
namespace comet {
namespace profiler {
using ProfileToken = u64;
constexpr ProfileToken kInvalidProfileToken{static_cast<ProfileToken>(-1)};

struct ProfileBackend {
  ProfileToken (*begin)(const schar* label);
  void (*end)(ProfileToken token);
};

// Register or unregister a static backend while profiling callbacks are quiescent.
void SetProfileBackend(const ProfileBackend* backend);

class ProfiledScope {
 public:
  explicit ProfiledScope(const schar* label);
  ProfiledScope(const ProfiledScope&) = delete;
  ProfiledScope(ProfiledScope&&) = delete;
  ProfiledScope& operator=(const ProfiledScope&) = delete;
  ProfiledScope& operator=(ProfiledScope&&) = delete;
  ~ProfiledScope();

 private:
  const ProfileBackend* backend_{nullptr};
  ProfileToken token_{kInvalidProfileToken};
};
}  // namespace profiler
}  // namespace comet

#define COMET_PROFILE(label) \
  comet::profiler::ProfiledScope COMET_CONCAT(_comet_profiler_, __LINE__){label}
#else
#define COMET_PROFILE(label) ((void)0)
#endif  // COMET_PROFILING

#endif  // COMET_CORE_PROFILER_PROFILER_H_
