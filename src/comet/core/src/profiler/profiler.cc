// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/profiler/profiler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_PROFILING
namespace comet {
namespace profiler {
namespace {
std::atomic<const ProfileBackend*> backend{nullptr};
thread_local bool in_callback{false};

struct CallbackGuard {
  CallbackGuard() { in_callback = true; }
  ~CallbackGuard() { in_callback = false; }
};
}  // namespace

void SetProfileBackend(const ProfileBackend* value) {
  backend.store(value, std::memory_order_release);
}

ProfiledScope::ProfiledScope(const schar* label) {
  if (in_callback) {
    return;
  }

  backend_ = backend.load(std::memory_order_acquire);
  if (backend_ != nullptr) {
    CallbackGuard guard{};
    token_ = backend_->begin(label);
  }
}

ProfiledScope::~ProfiledScope() {
  if (token_ != kInvalidProfileToken && !in_callback &&
      backend_ == backend.load(std::memory_order_acquire)) {
    CallbackGuard guard{};
    backend_->end(token_);
  }
}
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING
