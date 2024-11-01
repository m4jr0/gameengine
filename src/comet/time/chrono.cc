// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "chrono.h"

namespace comet {
namespace time {
void Chrono::Start(u32 duration_ms) {
  duration_ms_ = std::chrono::milliseconds(duration_ms);
  start_time_ = GetNow();
  is_finished_ = false;
}

void Chrono::Restart() {
  start_time_ = GetNow();
  is_finished_ = false;
}

bool Chrono::IsFinished() {
  if (is_finished_) {
    return true;
  }

  auto now = GetNow();

  if (now - start_time_ >= duration_ms_) {
    is_finished_ = true;
  }

  return is_finished_;
}

std::chrono::milliseconds Chrono::GetNow() noexcept {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now().time_since_epoch());
}
}  // namespace time
}  // namespace comet
