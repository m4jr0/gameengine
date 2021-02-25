// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "manager.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include <debug_windows.hpp>
#endif  // _WIN32

namespace koma {
void Manager::Initialize() {
  // Code has to be implemented in children.
}

void Manager::Destroy() {
  // Code has to be implemented in children.
}

void Manager::Update() {
  // Code has to be implemented in children.
}
}  // namespace koma
