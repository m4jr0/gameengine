// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "gid.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace gid {
Gid GenerateNewGeneration(Gid id) noexcept {
  Gid generation{GetGeneration(id) + 1};
  COMET_ASSERT(generation <= (kGenerationMask >> kIndexBits),
               "gid::GenerateNewGeneration", "generation is out of range",
               "generation", generation, "max_generation",
               (kGenerationMask >> kIndexBits));
  return (generation << kIndexBits) | GetIndex(id);
}

void InitializeGids() { internal::IdGenerationAllocator::Get().Initialize(); }

void DestroyGids() { internal::IdGenerationAllocator::Get().Destroy(); }

namespace internal {
memory::StatefulAllocator& IdGenerationAllocator::Get() {
  static IdGenerationAllocator singleton{512};
  return singleton;
}

IdGenerationAllocator::IdGenerationAllocator(usize base_capacity)
    : allocator_{sizeof(IdGeneration),
                 base_capacity / sizeof(IdGeneration) + sizeof(IdGeneration),
                 memory::kEngineMemoryTagGid} {}

IdGenerationAllocator::IdGenerationAllocator(
    IdGenerationAllocator&& other) noexcept
    : memory::StatefulAllocator{std::move(other)},
      allocator_{std::move(other.allocator_)} {}

IdGenerationAllocator& IdGenerationAllocator::operator=(
    IdGenerationAllocator&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  memory::Allocator::operator=(std::move(other));
  allocator_ = std::move(other.allocator_);
  return *this;
}

void* IdGenerationAllocator::AllocateAligned(usize size,
                                             memory::Alignment align) {
  return allocator_.AllocateAligned(size, align);
}

void IdGenerationAllocator::Deallocate(void* ptr) {
  allocator_.Deallocate(ptr);
}

void IdGenerationAllocator::OnInitialize() { allocator_.Initialize(); }

void IdGenerationAllocator::OnDestroy() { allocator_.Destroy(); }
}  // namespace internal

BreedHandler::BreedHandler()
    : generations_{&internal::IdGenerationAllocator::Get()} {}

BreedHandler::BreedHandler(BreedHandler&& other) noexcept
    : generations_{std::move(other.generations_)},
      free_ids_{std::move(other.free_ids_)} {}

BreedHandler& BreedHandler::operator=(BreedHandler&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  generations_ = std::move(other.generations_);
  free_ids_ = std::move(other.free_ids_);
  return *this;
}

void BreedHandler::Shutdown() {
  generations_.Release();
  free_ids_.clear();
}

Gid BreedHandler::Generate() {
  auto breed_id{gid::kInvalidId};

  if (free_ids_.size() > gid::kMinFreeIndices) {
    breed_id = free_ids_.front();

    COMET_ASSERT(!IsAlive(breed_id), "gid::BreedHandler::Generate",
                 "recycled breed id is still alive", "breed_id", breed_id);

    free_ids_.pop_front();
    breed_id = gid::GenerateNewGeneration(breed_id);
  } else {
    breed_id = static_cast<Gid>(generations_.GetSize());
    generations_.PushLast(0);
  }

  return breed_id;
}

void BreedHandler::Destroy(Gid breed_id) {
  COMET_ASSERT(IsAlive(breed_id), "gid::BreedHandler::Destroy",
               "breed is already destroyed", "breed_id", breed_id);

  const auto breed_index{GetIndex(breed_id)};

  COMET_ASSERT(breed_index < generations_.GetSize(),
               "gid::BreedHandler::Destroy", "breed id is malformed",
               "breed_id", breed_id, "breed_index", breed_index,
               "generation_count", generations_.GetSize());

  ++generations_[breed_index];
  free_ids_.push_back(breed_id);
}

bool BreedHandler::IsAlive(Gid breed_id) const {
  const auto breed_index{GetIndex(breed_id)};

  COMET_ASSERT(breed_index < generations_.GetSize(),
               "gid::BreedHandler::IsAlive", "breed id is malformed",
               "breed_id", breed_id, "breed_index", breed_index,
               "generation_count", generations_.GetSize());

  return generations_[breed_index] == GetGeneration(breed_id);
}
}  // namespace gid
}  // namespace comet
