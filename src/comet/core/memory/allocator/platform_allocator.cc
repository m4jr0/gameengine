// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "platform_allocator.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"

namespace comet {
namespace memory {
PlatformAllocator::PlatformAllocator(MemoryTag memory_tag)
    : memory_tag_{memory_tag} {}

void* PlatformAllocator::AllocateAligned(usize size, Alignment align) {
  return memory::AllocateAligned(size, align, memory_tag_);
}

void PlatformAllocator::Deallocate(void* ptr) { memory::Deallocate(ptr); }

PlatformStackAllocator::PlatformStackAllocator(usize capacity,
                                               MemoryTag memory_tag)
    : memory_tag_{memory_tag},
      capacity_{capacity},
      root_{nullptr},
      marker_{root_} {}

PlatformStackAllocator::PlatformStackAllocator(
    PlatformStackAllocator&& other) noexcept
    : StatefulAllocator{std::move(other)},
      memory_tag_{other.memory_tag_},
      capacity_{other.capacity_},
      root_{other.root_},
      marker_{other.marker_} {
  other.memory_tag_ = kEngineMemoryTagUntagged;
  other.capacity_ = 0;
  other.root_ = nullptr;
  other.marker_ = nullptr;
}

PlatformStackAllocator& PlatformStackAllocator::operator=(
    PlatformStackAllocator&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  if (root_ != nullptr) {
    memory::Deallocate(root_);
  }

  StatefulAllocator::operator=(other);
  memory_tag_ = other.memory_tag_;
  capacity_ = other.capacity_;
  root_ = other.root_;
  marker_ = other.marker_;

  other.memory_tag_ = kEngineMemoryTagUntagged;
  other.capacity_ = 0;
  other.root_ = nullptr;
  other.marker_ = nullptr;

  return *this;
}

void PlatformStackAllocator::Initialize() {
  StatefulAllocator::Initialize();
  COMET_ASSERT(capacity_ > 0, "Capacity is ", capacity_, "!");
  root_ = memory::AllocateMany<u8>(capacity_, memory_tag_);
  marker_ = root_;
}

void PlatformStackAllocator::Destroy() {
  StatefulAllocator::Destroy();
  memory::Deallocate(root_);
  root_ = nullptr;
  capacity_ = 0;
  marker_ = nullptr;
}

void* PlatformStackAllocator::AllocateAligned(usize size, Alignment align) {
  COMET_ASSERT(size > 0, "Allocation size provided is 0!");
  auto* p{AlignPointer(marker_, align)};
  COMET_ASSERT(p + size <= root_ + capacity_,
               "Could not allocate enough memory (", size, ")!");
  marker_ = p + size;
  return p;
}

void PlatformStackAllocator::Deallocate(void*) {
  // A stack allocator does not support individual deallocations, as it is
  // intended for temporary data only. Memory is only released when Clear() is
  // called, which resets the entire stack.
}

void PlatformStackAllocator::Clear() { marker_ = root_; }
}  // namespace memory
}  // namespace comet
