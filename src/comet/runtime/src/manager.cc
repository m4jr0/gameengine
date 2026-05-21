// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"
#include "comet/core/type_trait.h"

namespace comet {
namespace internal {
[[maybe_unused]]
static const schar* GetManagerStateLabel(ManagerState state) {
  switch (state) {
    case ManagerState::Uninitialized:
      return "uSninitialized";
    case ManagerState::Running:
      return "running";
    case ManagerState::ShutdownPending:
      return "shutdown pending";
    default:
      return kUnknownLabel;
  }
}
}  // namespace internal

Manager::~Manager() {
  COMET_ASSERT(state_ == ManagerState::Uninitialized, "Manager::~Manager",
               "manager is still initialized", "state",
               internal::GetManagerStateLabel(state_), "state_value",
               ToUnderlying(state_));
}

void Manager::Initialize() {
  COMET_ASSERT(state_ == ManagerState::Uninitialized, "Manager::Initialize",
               "manager is already initialized", "state",
               internal::GetManagerStateLabel(state_), "state_value",
               ToUnderlying(state_));
  OnInitialize();
  state_ = ManagerState::Running;
}

void Manager::PrepareShutdown() {
  COMET_ASSERT(state_ == ManagerState::Running, "Manager::PrepareShutdown",
               "manager is not running", "state",
               internal::GetManagerStateLabel(state_), "state_value",
               ToUnderlying(state_));
  OnPrepareShutdown();
  state_ = ManagerState::ShutdownPending;
}

void Manager::Shutdown() {
  COMET_ASSERT(state_ != ManagerState::Uninitialized, "Manager::Shutdown",
               "manager is not initialized", "state",
               internal::GetManagerStateLabel(state_), "state_value",
               ToUnderlying(state_));
  OnShutdown();
  state_ = ManagerState::Uninitialized;
}

void Manager::OnInitialize() {}

void Manager::OnPrepareShutdown() {}

void Manager::OnShutdown() {}

bool Manager::IsInitialized() const noexcept {
  return state_ != ManagerState::Uninitialized;
}

bool Manager::IsRunning() const noexcept {
  return state_ == ManagerState::Running;
}

bool Manager::IsShutdownPending() const noexcept {
  return state_ == ManagerState::ShutdownPending;
}

ManagerState Manager::GetState() const noexcept { return state_; }
}  // namespace comet