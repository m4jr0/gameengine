// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "manager.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
Manager::~Manager() {
  COMET_ASSERT(state_ == ManagerState::Uninitialized,
               "Destructor called for manager, but it is still initialized!");
}

void Manager::Initialize() {
  COMET_ASSERT(state_ == ManagerState::Uninitialized,
               "Tried to initialize manager, but it is already done!");
  OnInitialize();
  state_ = ManagerState::Running;
}

void Manager::PrepareShutdown() {
  COMET_ASSERT(state_ == ManagerState::Running,
               "Tried to prepare manager shutdown, but it is not running!");
  OnPrepareShutdown();
  state_ = ManagerState::ShutdownPending;
}

void Manager::Shutdown() {
  COMET_ASSERT(state_ != ManagerState::Uninitialized,
               "Tried to shutdown manager, but it is not initialized!");
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