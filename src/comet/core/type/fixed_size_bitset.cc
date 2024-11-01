// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "fixed_size_bitset.h"

namespace comet {
usize FixedSizeBitset::GetWordCountFromBitCount(usize bit_count) {
  return (bit_count + kWorkBitCount_ - 1) / kWorkBitCount_;
}

FixedSizeBitset::FixedSizeBitset(memory::AlignedAllocator* allocator,
                                 usize bit_count)
    : bit_count_{bit_count},
      word_count_{GetWordCountFromBitCount(bit_count_)},
      allocator_{allocator},
      words_{bit_count_ == 0
                 ? nullptr
                 : static_cast<Word*>(allocator_->AllocateAligned(
                       word_count_ * sizeof(Word), alignof(Word)))} {
  memory::ClearMemory(words_, word_count_ * sizeof(Word));
}

FixedSizeBitset::FixedSizeBitset(const FixedSizeBitset& other)
    : bit_count_{other.bit_count_}, allocator_{other.allocator_} {
  if (bit_count_ == 0) {
    words_ = nullptr;
    return;
  }

  words_ = static_cast<Word*>(
      allocator_->AllocateAligned(word_count_ * sizeof(Word), alignof(Word)));
  memory::CopyMemory(words_, other.words_, word_count_ * sizeof(Word));
}

FixedSizeBitset::FixedSizeBitset(FixedSizeBitset&& other) noexcept
    : bit_count_{other.bit_count_},
      word_count_{other.word_count_},
      allocator_{other.allocator_},
      words_{other.words_} {
  other.bit_count_ = 0;
  other.word_count_ = 0;
  other.allocator_ = nullptr;
  other.words_ = nullptr;
}

FixedSizeBitset& FixedSizeBitset::operator=(const FixedSizeBitset& other) {
  if (this == &other) {
    return *this;
  }

  if (words_ != nullptr) {
    Destroy();
  }

  bit_count_ = other.bit_count_;
  word_count_ = other.word_count_;
  allocator_ = other.allocator_;

  if (bit_count_ == 0) {
    words_ = nullptr;
    return *this;
  }

  words_ = static_cast<Word*>(
      allocator_->AllocateAligned(word_count_ * sizeof(Word), alignof(Word)));
  memory::CopyMemory(words_, other.words_, word_count_ * sizeof(Word));
  return *this;
}

FixedSizeBitset& FixedSizeBitset::operator=(FixedSizeBitset&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  if (words_ != nullptr) {
    Destroy();
  }

  bit_count_ = other.bit_count_;
  word_count_ = other.word_count_;
  allocator_ = other.allocator_;
  words_ = other.words_;

  other.bit_count_ = 0;
  other.word_count_ = 0;
  other.allocator_ = nullptr;
  other.words_ = nullptr;
  return *this;
}

FixedSizeBitset::~FixedSizeBitset() {
  if (words_ == nullptr) {
    return;
  }

  Destroy();
}

void FixedSizeBitset::Destroy() {
  COMET_ASSERT(words_ != nullptr, "Static array has already been destroyed!");

  allocator_->Deallocate(words_);
  words_ = nullptr;
  bit_count_ = 0;
  word_count_ = 0;
}

void FixedSizeBitset::Set(usize index) {
  COMET_ASSERT(index < bit_count_, "Index out of bounds: ", index,
               " >= ", bit_count_, "!");
  words_[index / kWorkBitCount_] |=
      (static_cast<Word>(1) << (index % kWorkBitCount_));
}

void FixedSizeBitset::Reset(usize index) {
  COMET_ASSERT(index < bit_count_, "Index out of bounds: ", index,
               " >= ", bit_count_, "!");
  words_[index / kWorkBitCount_] &=
      ~(static_cast<Word>(1) << (index % kWorkBitCount_));
}

bool FixedSizeBitset::Test(usize index) const {
  COMET_ASSERT(index < bit_count_, "Index out of bounds: ", index,
               " >= ", bit_count_, "!");
  return words_[index / kWorkBitCount_] &
         (static_cast<Word>(1) << (index % kWorkBitCount_));
}

void FixedSizeBitset::ResetAll() {
  memory::ClearMemory(words_, word_count_ * sizeof(Word));
}

bool FixedSizeBitset::operator[](usize index) const { return Test(index); }

usize FixedSizeBitset::GetSize() const noexcept { return bit_count_; }
}  // namespace comet