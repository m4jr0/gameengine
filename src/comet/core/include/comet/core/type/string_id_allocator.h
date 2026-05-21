// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_TYPE_STRING_ID_ALLOCATOR_H_
#define COMET_CORE_TYPE_STRING_ID_ALLOCATOR_H_

#include "comet/core/essentials.h"

#ifdef COMET_LABELIZE_STRING_IDS

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
#include <shared_mutex>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/container/map.h"
#include "comet/core/memory/allocator/stateful_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/string_id.h"
#endif  // COMET_LABELIZE_STRING_IDS

namespace comet {
namespace stringid {
#ifdef COMET_LABELIZE_STRING_IDS
namespace internal {
class StringIdAllocator : public memory::StatefulAllocator {
 public:
  StringIdAllocator() = delete;
  StringIdAllocator(usize capacity);
  StringIdAllocator(const StringIdAllocator&) = delete;
  StringIdAllocator(StringIdAllocator&&) = delete;
  StringIdAllocator& operator=(const StringIdAllocator&) = delete;
  StringIdAllocator& operator=(StringIdAllocator&&) = delete;
  ~StringIdAllocator() override = default;

  void* AllocateAligned(usize size, memory::Alignment align) override;
  void Deallocate(void*) override;

  // These functions are not thread-safe and must only be called during specific
  // synchronization points.
  void Clear();
  void Reset();

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  using StringIdAllocatorOffset = sptrdiff;
  static inline constexpr StringIdAllocatorOffset kInvalidOffset_{-1};

  usize capacity_{0};
  static_assert(
      std::atomic<StringIdAllocatorOffset>::is_always_lock_free,
      "std::atomic<StringIdAllocatorOffset> must be always lock-free");
  std::atomic<StringIdAllocatorOffset> offset_{kInvalidOffset_};
  u8* root_{nullptr};

#ifdef COMET_DEBUG_STRING_ID_ALLOCATOR
  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> must be always lock-free");
  std::atomic<usize> allocation_count_{0};
  std::atomic<usize> peak_used_size_{0};
  std::atomic<usize> clear_count_{0};
#endif  // COMET_DEBUG_STRING_ID_ALLOCATOR
};

struct DebugData {
  StringIdAllocator string_id_allocator{2097152};  // 2 MiB.
  // TODO(m4jr0): Consider lock-free solution.
  Map<StringId, schar*> label_table{};
  std::shared_mutex label_mutex{};

  void Initialize();
  void Destroy();
  bool IsInitialized() const noexcept;
  void InitializeIfNeeded();
};

DebugData& GetDebugData();
}  // namespace internal
#endif  // COMET_LABELIZE_STRING_IDS
}  // namespace stringid
}  // namespace comet

#endif  // COMET_CORE_TYPE_STRING_ID_ALLOCATOR_H_