// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MANAGER_H_
#define COMET_COMET_CORE_MANAGER_H_

#include "comet_precompile.h"

namespace comet {
class Manager {
 public:
  Manager() = default;
  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;
  Manager& operator=(const Manager&) = delete;
  Manager& operator=(Manager&&) = delete;
  virtual ~Manager() = default;

  virtual void Initialize();
  virtual void Destroy();
  virtual void Update();
};
}  // namespace comet

#endif  // COMET_COMET_CORE_MANAGER_H_
