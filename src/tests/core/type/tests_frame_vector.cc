// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_precompile.h"

#include "comet/core/memory/memory_manager.h"
#include "comet/core/type/stl_types.h"

#include "catch.hpp"

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"

namespace comet {
namespace comettests {
void GoToNextFrame() { comet::memory::MemoryManager::Get().Update(); }
}  // namespace comettests
}  // namespace comet

TEST_CASE("One-frame vectors creation", "[comet::memory]") {
  constexpr auto kU32Size{sizeof(comet::u32)};
  constexpr auto kElementCount{100};

  auto& configuration_manager{comet::conf::ConfigurationManager::Get()};
  configuration_manager.SetU32(
      comet::conf::kCoreOneFrameAllocatorCapacity,
      sizeof(
          comet::one_frame_unordered_map<comet::one_frame_string, comet::u32>) +
          kU32Size * kElementCount);
  configuration_manager.SetU32(
      comet::conf::kCoreTwoFrameAllocatorCapacity,
      sizeof(
          comet::one_frame_unordered_map<comet::one_frame_string, comet::u32>) +
          kU32Size * kElementCount * 2);

  auto& memory_manager{comet::memory::MemoryManager::Get()};
  comet::one_frame_vector<comet::u32> numbers{};
  memory_manager.Update();
  numbers.reserve(5);
  numbers.emplace_back(1);
  numbers.emplace_back(2);

  memory_manager.Update();
  comet::two_frame_unordered_map<comet::two_frame_string, comet::u32> map{};
  map["TEST"] = 32;

  memory_manager.Update();

  auto a{map.at("TEST")};
  ++a;

  memory_manager.Update();
  comet::two_frame_unordered_map<comet::two_frame_string, comet::u32> map2{};
  map2["TEST"] = 42;
  auto b{map.at("TEST")};
  ++a;
}