// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_TIME_CHRONO_H_
#define COMET_COMET_TIME_CHRONO_H_

#include <chrono>

#include "comet/core/essentials.h"

namespace comet {
namespace time {
class Chrono {
 public:
  Chrono() = default;
  Chrono(const Chrono&) = default;
  Chrono(Chrono&&) = default;
  Chrono& operator=(const Chrono&) = default;
  Chrono& operator=(Chrono&&) = default;
  ~Chrono() = default;

  void Start(u32 duration_ms);
  void Restart();
  bool IsFinished();

 private:
  std::chrono::steady_clock::time_point start_time_{};
  std::chrono::milliseconds duration_ms_{0};
  bool is_finished_{false};
};
}  // namespace time
}  // namespace comet

#endif  // COMET_COMET_TIME_CHRONO_H_
