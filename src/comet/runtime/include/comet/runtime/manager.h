// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_MANAGER_H_
#define COMET_RUNTIME_MANAGER_H_

#include "comet/core/essentials.h"

namespace comet {
enum class ManagerState : u8 {
  Uninitialized,
  Running,
  ShutdownPending,
};

class Manager {
 public:
  Manager() = default;
  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;
  Manager& operator=(const Manager&) = delete;
  Manager& operator=(Manager&&) = delete;
  virtual ~Manager();

  void Initialize();
  void PrepareShutdown();
  void Shutdown();

  bool IsInitialized() const noexcept;
  bool IsRunning() const noexcept;
  bool IsShutdownPending() const noexcept;
  ManagerState GetState() const noexcept;

 protected:
  virtual void OnInitialize();
  virtual void OnPrepareShutdown();
  virtual void OnShutdown();

 protected:
  ManagerState state_{ManagerState::Uninitialized};
};
}  // namespace comet

#endif  // COMET_RUNTIME_MANAGER_H_