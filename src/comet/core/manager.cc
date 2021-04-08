// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "manager.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace core {
void Manager::Initialize() {
  // Code has to be implemented in children.
}

void Manager::Destroy() {
  // Code has to be implemented in children.
}

void Manager::Update() {
  // Code has to be implemented in children.
}
}  // namespace core
}  // namespace comet
