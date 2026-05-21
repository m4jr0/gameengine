// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_CONTAINER_BITSET_H_
#define COMET_CORE_CONTAINER_BITSET_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"

namespace comet {
class Bitset {
  struct WithSizeTag {};

 public:
  using Word = u64;

  static usize GetWordCountFromBitCount(usize bit_count);

  static Bitset WithSize(memory::Allocator* allocator, usize bit_count) {
    return Bitset{allocator, WithSizeTag{}, bit_count};
  }

  Bitset() = default;
  explicit Bitset(memory::Allocator* allocator);
  Bitset(const Bitset& other);
  Bitset(Bitset&& other) noexcept;
  Bitset& operator=(const Bitset& other);
  Bitset& operator=(Bitset&& other) noexcept;
  ~Bitset();

  void Release();

  void Set(usize index);
  void Reset(usize index);
  bool Test(usize index) const;
  void ResetAll();

  void Resize(usize new_bit_count);
  void Clear();

  bool operator[](usize index) const;

  usize GetSize() const noexcept;
  usize GetWordCount() const noexcept;
  memory::Allocator* GetAllocator() noexcept;
  const memory::Allocator* GetAllocator() const noexcept;

 private:
  Bitset(memory::Allocator* allocator, WithSizeTag, usize bit_count);

  static inline constexpr usize kWordBitCount_{sizeof(Word) * kCharBit};
  static_assert((kWordBitCount_ & (kWordBitCount_ - 1)) == 0,
                "kWordBitCount_ must be a power of 2");

  usize bit_count_{0};
  usize word_count_{0};
  memory::Allocator* allocator_{nullptr};
  Word* words_{nullptr};
};
}  // namespace comet

#endif  // COMET_CORE_CONTAINER_BITSET_H_