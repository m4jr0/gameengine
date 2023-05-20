// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_precompile.h"

#include "comet/core/memory/memory_manager.h"
#include "comet/core/type/structure/stl_types.h"

#include "catch.hpp"

namespace comet {
namespace comettests {
void GoToNextFrame() { comet::memory::MemoryManager::Get().Update(); }
}  // namespace comettests
}  // namespace comet

TEST_CASE("One-frame vectors creation", "[comet::memory]") {
  auto& memory_manager{comet::memory::MemoryManager::Get()};
  memory_manager.Initialize();
  comet::one_frame_vector<comet::u32> numbers{};
}
