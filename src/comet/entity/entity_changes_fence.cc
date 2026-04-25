// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "entity_changes_fence.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/entity/entity_manager.h"

namespace comet {
namespace entity {
void EntityChangesFence::Reset() noexcept {
  state_ = EntityChangesFenceState::Idle;
  target_generation_ = 0;
}

void EntityChangesFence::Begin() noexcept {
  const auto current_generation{EntityManager::Get().GetUpdateGeneration()};
  target_generation_ = current_generation + 1;
  state_ = EntityChangesFenceState::Waiting;
}

bool EntityChangesFence::IsIdle() const noexcept {
  return state_ == EntityChangesFenceState::Idle;
  ;
}

bool EntityChangesFence::IsWaiting() const noexcept {
  return state_ == EntityChangesFenceState::Waiting;
}

bool EntityChangesFence::HasPassed() const noexcept {
  return state_ == EntityChangesFenceState::Passed;
}

EntityChangesFenceState EntityChangesFence::GetState() const noexcept {
  return state_;
}

usize EntityChangesFence::GetTargetGeneration() const noexcept {
  return target_generation_;
}

bool EntityChangesFence::Poll() noexcept {
  if (state_ == EntityChangesFenceState::Passed) {
    return true;
  }

  if (state_ == EntityChangesFenceState::Idle) {
    return false;
  }

  const auto current_generation{EntityManager::Get().GetUpdateGeneration()};

  if (current_generation < target_generation_) {
    return false;
  }

  state_ = EntityChangesFenceState::Passed;
  return true;
}
}  // namespace entity
}  // namespace comet
