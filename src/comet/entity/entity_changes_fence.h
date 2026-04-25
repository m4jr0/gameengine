// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_CHANGES_FENCE_H_
#define COMET_COMET_ENTITY_ENTITY_CHANGES_FENCE_H_

// External. ///////////////////////////////////////////////////////////////////
#include <concepts>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/continuation/continuation_utils.h"
#include "comet/core/continuation/type/continuation_phase.h"
#include "comet/core/essentials.h"

namespace comet {
namespace entity {
enum class EntityChangesFenceState {
  Idle,
  Waiting,
  Passed,
};

class EntityChangesFence {
 public:
  EntityChangesFence() = default;
  EntityChangesFence(const EntityChangesFence&) = default;
  EntityChangesFence(EntityChangesFence&&) noexcept = default;
  EntityChangesFence& operator=(const EntityChangesFence&) = default;
  EntityChangesFence& operator=(EntityChangesFence&&) noexcept = default;
  ~EntityChangesFence() = default;

  void Reset() noexcept;
  void Begin() noexcept;

  bool IsIdle() const noexcept;
  bool IsWaiting() const noexcept;
  bool HasPassed() const noexcept;

  EntityChangesFenceState GetState() const noexcept;
  usize GetTargetGeneration() const noexcept;

  bool Poll() noexcept;

 private:
  EntityChangesFenceState state_{EntityChangesFenceState::Idle};
  usize target_generation_{0};
};

template <typename T>
concept EntityCallback = requires(T callback) {
  { callback() } -> std::same_as<void>;
};

template <PollableFence Fence, EntityCallback Callback>
void ThenAfterEntityChanges(
    Fence&& first_fence, Callback&& callback,
    ContinuationPhase phase = ContinuationPhase::AfterEndFrame) {
  Then(
      std::forward<Fence>(first_fence),
      [callback = std::forward<Callback>(callback), phase]() mutable {
        EntityChangesFence entity_fence{};
        entity_fence.Begin();

        Then(std::move(entity_fence), std::move(callback), phase);
      },
      phase);
}

template <EntityCallback Callback>
void AfterEntityChanges(
    Callback&& callback,
    ContinuationPhase phase = ContinuationPhase::AfterEndFrame) {
  EntityChangesFence entity_fence{};
  entity_fence.Begin();

  Then(std::move(entity_fence), std::forward<Callback>(callback), phase);
}
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_ENTITY_CHANGES_FENCE_H_
