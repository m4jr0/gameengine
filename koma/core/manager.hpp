// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_MANAGER_HPP_
#define KOMA_CORE_MANAGER_HPP_

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace koma {
class Manager {
 public:
  virtual void Initialize();
  virtual void Destroy();
  virtual void Update();
};
};  // namespace koma

#endif  // KOMA_CORE_MANAGER_HPP_
