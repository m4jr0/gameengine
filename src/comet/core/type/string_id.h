// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_STRING_ID_H_
#define COMET_COMET_CORE_TYPE_STRING_ID_H_

#include <atomic>

#include "comet/core/essentials.h"
#ifdef COMET_LABELIZE_STRING_IDS
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/memory.h"
#endif  // COMET_LABELIZE_STRING_IDS

namespace comet {
namespace stringid {
using StringId = u32;
constexpr auto kInvalidStringId{static_cast<StringId>(-1)};

#ifdef COMET_LABELIZE_STRING_IDS
namespace internal {
class StringIdAllocator : public memory::Allocator {
 public:
  StringIdAllocator() = delete;
  StringIdAllocator(usize capacity);
  StringIdAllocator(const StringIdAllocator&) = delete;
  StringIdAllocator(StringIdAllocator&&) = delete;
  StringIdAllocator& operator=(const StringIdAllocator&) = delete;
  StringIdAllocator& operator=(StringIdAllocator&&) = delete;
  ~StringIdAllocator();

  void Initialize();
  void Destroy();

  void* AllocateAligned(usize size, memory::Alignment align) override;
  void Deallocate(void*) override;

  // These functions are not thread-safe and must only be called during specific
  // synchronization points.
  void Clear();
  void Reset();

  bool IsInitialized() const noexcept;

 private:
  using StringIdAllocatorOffset = sptrdiff;
  static inline constexpr StringIdAllocatorOffset kInvalidOffset_{-1};

  usize capacity_{0};
  static_assert(std::atomic<StringIdAllocatorOffset>::is_always_lock_free,
                "std::atomic<StringIdAllocatorOffset> needs to be always "
                "lock-free. Unsupported "
                "architecture");
  std::atomic<StringIdAllocatorOffset> offset_{kInvalidOffset_};
  u8* root_{nullptr};
};
}  // namespace internal
#endif  // COMET_LABELIZE_STRING_IDS

class StringIdHandler {
 public:
  StringIdHandler() = default;
  StringIdHandler(const StringIdHandler&) = delete;
  StringIdHandler(StringIdHandler&&) = delete;
  StringIdHandler& operator=(const StringIdHandler&) = delete;
  StringIdHandler& operator=(StringIdHandler&&) = delete;
  ~StringIdHandler();

  StringId Generate(const schar* str, usize length);
  StringId Generate(const wchar* str, usize length);
  StringId Generate(const wchar* str);
  StringId Generate(const schar* str);

  // Return temporary string for debug purposes. The schar* returned SHOULD NOT
  // be stored.
  const schar* Labelize(StringId string_id) const;
};

extern StringIdHandler* SetHandler(bool is_destroy = false);
}  // namespace stringid
}  // namespace comet

#define COMET_STRING_ID(str) comet::stringid::SetHandler()->Generate(str)
// Return temporary string for debug purposes. The schar* returned SHOULD NOT be
// stored.
#define COMET_STRING_ID_LABEL(string_id) \
  comet::stringid::SetHandler()->Labelize(string_id)
#define COMET_STRING_ID_DESTROY() comet::stringid::SetHandler(true)

#endif  // COMET_COMET_CORE_TYPE_STRING_ID_H_