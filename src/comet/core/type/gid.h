// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_GID_H_
#define COMET_COMET_CORE_TYPE_GID_H_

#include <deque>

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/array.h"

namespace comet {
namespace gid {
using Gid = u32;

constexpr u32 kGenerationBits{8};
constexpr u32 kIndexBit{sizeof(Gid) * 8 - kGenerationBits};
constexpr Gid kGenerationMask{((Gid{1} << kGenerationBits) - 1) << kIndexBit};
constexpr Gid kIndexMask{(Gid{1} << kIndexBit) - 1};
constexpr Gid kIdMask{Gid{static_cast<u32>(-1)}};  // Set all the bits to 1.
constexpr auto kInvalidId{kIdMask};
constexpr u32 kMinFreeIndices{1024};

using IdGeneration =
    std::conditional_t<kGenerationBits <= 16,
                       std::conditional_t<kGenerationBits <= 8, u8, u16>, u32>;

static_assert(sizeof(IdGeneration) * 8 >= kGenerationBits);
static_assert(sizeof(Gid) - sizeof(IdGeneration) > 0);

constexpr bool IsValid(Gid id) noexcept { return id != kIdMask; }
constexpr Gid GetIndex(Gid id) noexcept { return id & kIdMask; }
constexpr Gid GetGeneration(Gid id) noexcept { return id & kGenerationMask; }
Gid GenerateNewGeneration(Gid id) noexcept;

namespace internal {
class IdGenerationAllocator : public memory::Allocator {
 public:
  static memory::Allocator& Get();

  IdGenerationAllocator() = default;
  explicit IdGenerationAllocator(usize base_capacity);
  IdGenerationAllocator(const IdGenerationAllocator&) = delete;
  IdGenerationAllocator(IdGenerationAllocator&& other) noexcept;
  IdGenerationAllocator& operator=(const IdGenerationAllocator&) = delete;
  IdGenerationAllocator& operator=(IdGenerationAllocator&& other) noexcept;
  ~IdGenerationAllocator() = default;

  void Initialize();
  void Destroy();

  void* AllocateAligned(usize size, memory::Alignment align) override;
  void Deallocate(void* ptr) override;

 private:
  memory::FiberFreeListAllocator allocator_{};
};
}  // namespace internal

void InitializeGids();
void DestroyGids();

class BreedHandler {
 public:
  BreedHandler();
  BreedHandler(const BreedHandler&) = delete;
  BreedHandler(BreedHandler&& other) noexcept;
  BreedHandler& operator=(const BreedHandler&) = delete;
  BreedHandler& operator=(BreedHandler&& other) noexcept;
  ~BreedHandler() = default;

  void Shutdown();

  Gid Generate();
  bool IsAlive(Gid breed_id) const;
  void Destroy(Gid breed_id);

 private:
  Array<gid::IdGeneration> generations_{};
  std::deque<Gid> free_ids_{};
};
}  // namespace gid
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_GID_H_