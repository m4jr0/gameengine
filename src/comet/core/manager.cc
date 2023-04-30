// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be it in the LICENSE file.

#include "manager.h"

namespace comet {
Manager::Manager(const ManagerDescr& descr) {}

Manager ::~Manager() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for manager, but it is still initialized!");
}

void Manager ::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize manager, but it is already done!");
  is_initialized_ = true;
}

void Manager ::Shutdown() {
  COMET_ASSERT(is_initialized_,
               "Tried to shutdown manager, but it is not initialized!");
  is_initialized_ = false;
}

bool Manager::IsInitialized() const noexcept { return is_initialized_; }
}  // namespace comet
