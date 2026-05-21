// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_CONTINUATION_CONTINUATION_UTILS_H_
#define COMET_RUNTIME_CONTINUATION_CONTINUATION_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include <concepts>
#include <type_traits>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/continuation/continuation_manager.h"
#include "comet/core/continuation/type/continuation_phase.h"
#include "comet/core/essentials.h"

namespace comet {
template <typename T>
concept PollableFence = requires(T fence) {
  { fence.Poll() } -> std::convertible_to<bool>;
};

template <typename T>
concept VoidCallback = requires(T callback) {
  { callback() } -> std::same_as<void>;
};

template <PollableFence Fence, VoidCallback Callback>
class ThenContinuation {
 public:
  ThenContinuation(Fence&& fence, Callback&& callback)
      : fence_{std::forward<Fence>(fence)},
        callback_{std::forward<Callback>(callback)} {}

  ThenContinuation(const ThenContinuation&) = delete;
  ThenContinuation(ThenContinuation&&) noexcept = default;
  ThenContinuation& operator=(const ThenContinuation&) = delete;
  ThenContinuation& operator=(ThenContinuation&&) noexcept = default;
  ~ThenContinuation() = default;

  bool Poll() {
    if (!fence_.Poll()) {
      return false;
    }

    callback_();
    return true;
  }

 private:
  std::decay_t<Fence> fence_{};
  std::decay_t<Callback> callback_{};
};

template <PollableFence Fence, VoidCallback Callback>
void Then(Fence&& fence, Callback&& callback,
          ContinuationPhase phase = ContinuationPhase::AfterEndFrame) {
  using ContinuationType = ThenContinuation<Fence, Callback>;

  ContinuationManager::Get().Submit(
      phase, ContinuationType{std::forward<Fence>(fence),
                              std::forward<Callback>(callback)});
}
}  // namespace comet

#endif  // COMET_RUNTIME_CONTINUATION_CONTINUATION_UTILS_H_