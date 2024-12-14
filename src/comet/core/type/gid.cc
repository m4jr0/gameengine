// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "gid.h"

namespace comet {
namespace gid {
Gid GenerateNewGeneration(Gid id) noexcept {
  const Gid generation{GetGeneration(id) + 1};
  COMET_ASSERT(generation >= (kGenerationMask >> kIndexBit),
               "Bad generation retrieved: ", generation, "!");
  return generation | GetIndex(id);
}

void InitializeGids() { internal::IdGenerationAllocator::Get().Initialize(); }

void DestroyGids() { internal::IdGenerationAllocator::Get().Destroy(); }

namespace internal {
memory::Allocator& IdGenerationAllocator::Get() {
  static IdGenerationAllocator singleton{512};
  return singleton;
}

IdGenerationAllocator::IdGenerationAllocator(usize base_capacity)
    : allocator_{sizeof(IdGeneration),
                 base_capacity / sizeof(IdGeneration) + sizeof(IdGeneration),
                 memory::kEngineMemoryTagGid} {}

IdGenerationAllocator::IdGenerationAllocator(
    IdGenerationAllocator&& other) noexcept
    : memory::Allocator{std::move(other)},
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

void IdGenerationAllocator::Initialize() {
  memory::Allocator::Initialize();
  allocator_.Initialize();
}

void IdGenerationAllocator::Destroy() {
  allocator_.Destroy();
  memory::Allocator::Destroy();
}

void* IdGenerationAllocator::AllocateAligned(usize size,
                                             memory::Alignment align) {
  return allocator_.AllocateAligned(size, align);
}

void IdGenerationAllocator::Deallocate(void* ptr) {
  allocator_.Deallocate(ptr);
}
}  // namespace internal

BreedHandler::BreedHandler()
    : generations_{&internal::IdGenerationAllocator::Get()} {}

BreedHandler::BreedHandler(BreedHandler&& other) noexcept
    : generations_{std::move(other.generations_)},
      free_ids_{std::move(other.free_ids_)} {
  other.generations_.Clear();
  other.free_ids_.clear();
}

BreedHandler& BreedHandler::operator=(BreedHandler&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  generations_ = std::move(other.generations_);
  free_ids_ = std::move(other.free_ids_);
  other.generations_.Clear();
  other.free_ids_.clear();
  return *this;
}

void BreedHandler::Shutdown() {
  generations_.Clear();
  free_ids_.clear();
}

Gid BreedHandler::Generate() {
  auto breed_id{gid::kInvalidId};

  if (free_ids_.size() > gid::kMinFreeIndices) {
    breed_id = free_ids_.front();
    COMET_ASSERT(!IsAlive(breed_id), "Breed ID generated is alive: ", breed_id,
                 "!");
    free_ids_.pop_front();
    breed_id = gid::GenerateNewGeneration(breed_id);
  } else {
    breed_id = static_cast<Gid>(generations_.GetSize());
    generations_.PushBack(0);
  }

  return breed_id;
}

bool BreedHandler::IsAlive(Gid breed_id) const {
  const auto breed_index{GetIndex(breed_id)};
  COMET_ASSERT(breed_index < generations_.GetSize(), "Breed with ID ", breed_id,
               " is malformed.");
  return generations_[breed_index] == GetGeneration(breed_id);
}

void BreedHandler::Destroy(Gid breed_id) {
  COMET_ASSERT(IsAlive(breed_id), "Breed with ID ", breed_id,
               " is already destroyed.");
  const auto breed_index{GetIndex(breed_id)};
  COMET_ASSERT(breed_index < generations_.GetSize(), "Breed with ID ", breed_id,
               " is malformed.");
  ++generations_[breed_index];
  free_ids_.push_back(breed_id);
}
}  // namespace gid
}  // namespace comet
