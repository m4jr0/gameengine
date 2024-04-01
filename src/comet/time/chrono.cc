// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "chrono.h"

namespace comet {
namespace time {
void Chrono::Start(u32 duration_ms) {
  duration_ms_ = std::chrono::milliseconds(duration_ms);
  start_time_ = std::chrono::steady_clock::now();
  is_finished_ = false;
}

void Chrono::Restart() {
  start_time_ = std::chrono::steady_clock::now();
  is_finished_ = false;
}

bool Chrono::IsFinished() {
  if (is_finished_) {
    return true;
  }

  auto now = std::chrono::steady_clock::now();
  if (now - start_time_ >= duration_ms_) {
    is_finished_ = true;
  }

  return is_finished_;
}
}  // namespace time
}  // namespace comet
