// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/id/gid_pool.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace gid {
GidPool::GidPool(memory::Allocator* allocator) { Initialize(allocator); }

GidPool::GidPool(GidPool&& other) noexcept
    : allocator_{other.allocator_},
      generations_{std::move(other.generations_)},
      free_ids_{std::move(other.free_ids_)} {
  other.allocator_ = nullptr;
}

GidPool& GidPool::operator=(GidPool&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Shutdown();

  allocator_ = other.allocator_;
  generations_ = std::move(other.generations_);
  free_ids_ = std::move(other.free_ids_);

  other.allocator_ = nullptr;
  return *this;
}

void GidPool::Initialize(memory::Allocator* allocator) {
  COMET_ASSERT(allocator != nullptr, "gid::GidPool::Initialize",
               "allocator is null");
  COMET_ASSERT(allocator_ == nullptr, "gid::GidPool::Initialize",
               "gid pool is already initialized");

  allocator_ = allocator;
  generations_ = Array<IdGeneration>{allocator_};
  free_ids_ = Array<Gid>{allocator_};
}

void GidPool::Shutdown() {
  generations_.Release();
  free_ids_.Release();
  allocator_ = nullptr;
}

Gid GidPool::Generate() {
  COMET_ASSERT(allocator_ != nullptr, "gid::GidPool::Generate",
               "gid pool is not initialized");

  Gid index;

  if (free_ids_.GetSize() > kMinFreeIndices) {
    index = free_ids_.GetLast();
    free_ids_.PopLast();
  } else {
    index = static_cast<Gid>(generations_.GetSize());
    generations_.PushLast(0);
  }

  COMET_ASSERT(index < generations_.GetSize(), "gid::GidPool::Generate",
               "index is out of bounds", "index", index, "generation_count",
               generations_.GetSize());

  return gid::Generate(index, generations_[index]);
}

void GidPool::Destroy(Gid id) {
  COMET_ASSERT(IsValid(id), "gid::GidPool::Destroy", "id is invalid");
  COMET_ASSERT(IsAlive(id), "gid::GidPool::Destroy", "id is not alive", "id",
               id);

  const auto index{GetIndex(id)};
  generations_[index] = static_cast<IdGeneration>(
      (generations_[index] + 1) & ((Gid{1} << kGenerationBits) - 1));
  free_ids_.PushLast(index);
}

bool GidPool::IsAlive(Gid id) const {
  if (!IsValid(id)) {
    return false;
  }

  const auto index{GetIndex(id)};

  if (index >= generations_.GetSize()) {
    return false;
  }

  return generations_[index] == GetGeneration(id);
}
}  // namespace gid
}  // namespace comet