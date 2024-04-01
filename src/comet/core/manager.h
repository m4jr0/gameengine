// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MANAGER_H_
#define COMET_COMET_CORE_MANAGER_H_

#include "comet/core/essentials.h"

namespace comet {
class Manager {
 public:
  Manager() = default;
  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;
  Manager& operator=(const Manager&) = delete;
  Manager& operator=(Manager&&) = delete;
  virtual ~Manager();

  virtual void Initialize();
  virtual void Shutdown();

  bool IsInitialized() const noexcept;

 protected:
  bool is_initialized_{false};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_MANAGER_H_
