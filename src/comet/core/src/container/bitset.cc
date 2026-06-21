// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/container/bitset.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/runtime/memory/tagged_memory.h"

namespace comet {
usize Bitset::GetWordCountFromBitCount(usize bit_count) {
  return (bit_count + kWordBitCount_ - 1) / kWordBitCount_;
}

Bitset::Bitset(memory::Allocator* allocator) : allocator_{allocator} {}

Bitset::Bitset(memory::Allocator* allocator, WithSizeTag, usize bit_count)
    : bit_count_{bit_count},
      word_count_{GetWordCountFromBitCount(bit_count_)},
      allocator_{allocator},
      words_{bit_count_ == 0
                 ? nullptr
                 : static_cast<Word*>(allocator->AllocateAligned(
                       word_count_ * sizeof(Word), alignof(Word)))} {
  COMET_ASSERT(bit_count_ == 0 || allocator_ != nullptr, "Bitset::WithSize",
               "allocator is null");

  if (words_ != nullptr) {
    memory::ClearMemory(words_, word_count_ * sizeof(Word));
  }
}

Bitset::Bitset(const Bitset& other)
    : bit_count_{other.bit_count_},
      word_count_{other.word_count_},
      allocator_{other.allocator_} {
  COMET_ASSERT(bit_count_ == 0 || allocator_ != nullptr, "Bitset::Bitset",
               "source allocator is null");

  if (bit_count_ == 0) {
    return;
  }

  words_ = static_cast<Word*>(
      allocator_->AllocateAligned(word_count_ * sizeof(Word), alignof(Word)));
  memory::CopyMemory(words_, other.words_, word_count_ * sizeof(Word));
}

Bitset::Bitset(Bitset&& other) noexcept
    : bit_count_{other.bit_count_},
      word_count_{other.word_count_},
      allocator_{other.allocator_},
      words_{other.words_} {
  other.bit_count_ = 0;
  other.word_count_ = 0;
  other.allocator_ = nullptr;
  other.words_ = nullptr;
}

Bitset& Bitset::operator=(const Bitset& other) {
  if (this == &other) {
    return *this;
  }

  Release();

  bit_count_ = other.bit_count_;
  word_count_ = other.word_count_;
  allocator_ = other.allocator_;

  COMET_ASSERT(bit_count_ == 0 || allocator_ != nullptr,
               "Bitset::operator=", "source allocator is null");

  if (bit_count_ == 0) {
    return *this;
  }

  words_ = static_cast<Word*>(
      allocator_->AllocateAligned(word_count_ * sizeof(Word), alignof(Word)));
  memory::CopyMemory(words_, other.words_, word_count_ * sizeof(Word));
  return *this;
}

Bitset& Bitset::operator=(Bitset&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Release();

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

Bitset::~Bitset() { Release(); }

void Bitset::Release() {
  if (words_ != nullptr) {
    COMET_ASSERT(allocator_ != nullptr, "Bitset::Release",
                 "allocator is null while words are not null");
    allocator_->Deallocate(words_);
  }

  bit_count_ = 0;
  word_count_ = 0;
  allocator_ = nullptr;
  words_ = nullptr;
}

void Bitset::Set(usize index) {
  COMET_ASSERT(index < bit_count_, "Bitset::Set", "index out of bounds",
               "index", index, "bit_count", bit_count_);

  words_[index / kWordBitCount_] |= static_cast<Word>(1)
                                    << (index % kWordBitCount_);
}

void Bitset::Reset(usize index) {
  COMET_ASSERT(index < bit_count_, "Bitset::Reset", "index out of bounds",
               "index", index, "bit_count", bit_count_);

  words_[index / kWordBitCount_] &=
      ~(static_cast<Word>(1) << (index % kWordBitCount_));
}

bool Bitset::Test(usize index) const {
  COMET_ASSERT(index < bit_count_, "Bitset::Test", "index out of bounds",
               "index", index, "bit_count", bit_count_);

  return (words_[index / kWordBitCount_] &
          (static_cast<Word>(1) << (index % kWordBitCount_))) != 0;
}

void Bitset::ResetAll() {
  if (words_ == nullptr) {
    return;
  }

  memory::ClearMemory(words_, word_count_ * sizeof(Word));
}

void Bitset::Resize(usize new_bit_count) {
  if (new_bit_count <= bit_count_) {
    return;
  }

  COMET_ASSERT(allocator_ != nullptr, "Bitset::Resize", "allocator is null");

  const auto old_bit_count{bit_count_};
  const auto new_word_count{GetWordCountFromBitCount(new_bit_count)};

  auto* new_words{static_cast<Word*>(allocator_->AllocateAligned(
      new_word_count * sizeof(Word), alignof(Word)))};

  memory::ClearMemory(new_words, new_word_count * sizeof(Word));

  if (words_ != nullptr) {
    const auto full_old_words{old_bit_count / kWordBitCount_};
    memory::CopyMemory(new_words, words_, full_old_words * sizeof(Word));

    const auto remaining_bits{old_bit_count % kWordBitCount_};

    if (remaining_bits != 0) {
      const auto mask{(Word{1} << remaining_bits) - 1};
      new_words[full_old_words] = words_[full_old_words] & mask;
    }

    allocator_->Deallocate(words_);
  }

  words_ = new_words;
  bit_count_ = new_bit_count;
  word_count_ = new_word_count;
}

void Bitset::Clear() {
  if (words_ != nullptr) {
    memory::ClearMemory(words_, word_count_ * sizeof(Word));
  }
}

bool Bitset::operator[](usize index) const { return Test(index); }

usize Bitset::GetSize() const noexcept { return bit_count_; }

usize Bitset::GetWordCount() const noexcept { return word_count_; }

memory::Allocator* Bitset::GetAllocator() noexcept { return allocator_; }

const memory::Allocator* Bitset::GetAllocator() const noexcept {
  return allocator_;
}
}  // namespace comet