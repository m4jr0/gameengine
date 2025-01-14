// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_BITSET_H_
#define COMET_COMET_CORE_TYPE_BITSET_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"

namespace comet {
class Bitset {
 public:
  using Word = u64;

  static usize GetWordCountFromBitCount(usize bit_count);

  Bitset() = default;
  Bitset(memory::Allocator* allocator, usize bit_count);
  Bitset(const Bitset& other);
  Bitset(Bitset&& other) noexcept;
  Bitset& operator=(const Bitset& other);
  Bitset& operator=(Bitset&& other) noexcept;
  ~Bitset();

  void Set(usize index);
  void Reset(usize index);
  bool Test(usize index) const;
  void ResetAll();
  void Clear();

  bool operator[](usize index) const;

  usize GetSize() const noexcept;

 private:
  static inline constexpr usize kWorkBitCount_{sizeof(Word) * kCharBit};
  static_assert((kWorkBitCount_ & (kWorkBitCount_ - 1)) == 0,
                "kWorkdBitCount_ must be a power of 2!");

  usize bit_count_{0};
  usize word_count_{0};
  memory::Allocator* allocator_{nullptr};
  Word* words_{nullptr};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_BITSET_H_