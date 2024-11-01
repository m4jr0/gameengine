// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_FIXED_SIZE_ARRAY_H_
#define COMET_COMET_CORE_TYPE_FIXED_SIZE_ARRAY_H_

#include <utility>

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/aligned_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/iterator.h"

namespace comet {
template <typename T>
class FixedSizeArray {
 public:
  COMET_POPULATE_ITERATOR(ContiguousIterator<T>, ConstContiguousIterator<T>,
                          data_, size_)

  FixedSizeArray() = default;

  FixedSizeArray(memory::AlignedAllocator* allocator, usize size)
      : size_{size},
        allocator_{allocator},
        data_{size_ == 0 ? nullptr
                         : static_cast<T*>(allocator_->AllocateAligned(
                               size_ * sizeof(T), alignof(T)))} {
    for (usize i{0}; i < size_; ++i) {
      new (&data_[i]) T{};
    }
  }

  template <typename... Targs>
  FixedSizeArray(memory::AlignedAllocator* allocator, Targs&&... args)
      : size_{sizeof...(Targs)},
        allocator_{allocator},
        data_{size_ == 0 ? nullptr
                         : static_cast<T*>(allocator_->AllocateAligned(
                               size_ * sizeof(T), alignof(T)))} {
    usize index{0};
    ((new (&data_[index++]) T{std::forward<T>(args)}), ...);
  }

  FixedSizeArray(const FixedSizeArray& other)
      : size_{other.size_}, allocator_{other.allocator_} {
    if (size_ == 0) {
      data_ = nullptr;
      return;
    }

    data_ = static_cast<T*>(
        allocator_->AllocateAligned(size_ * sizeof(T), alignof(T)));
    memory::CopyMemory(data_, other.data_, size_ * sizeof(T));
  }

  FixedSizeArray(FixedSizeArray&& other) noexcept
      : size_{other.size_}, allocator_{other.allocator_}, data_{other.data_} {
    other.size_ = 0;
    other.allocator_ = nullptr;
    other.data_ = nullptr;
  }

  FixedSizeArray& operator=(const FixedSizeArray& other) {
    if (this == &other) {
      return *this;
    }

    if (data_ != nullptr) {
      Destroy();
    }

    size_ = other.size_;
    allocator_ = other.allocator_;

    if (size_ == 0) {
      data_ = nullptr;
      return *this;
    }

    data_ = static_cast<T*>(
        allocator_->AllocateAligned(size_ * sizeof(T), alignof(T)));
    memory::CopyMemory(data_, other.data_, size_ * sizeof(T));
    return *this;
  }

  FixedSizeArray& operator=(FixedSizeArray&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    if (data_ != nullptr) {
      Destroy();
    }

    size_ = other.size_;
    allocator_ = other.allocator_;
    data_ = other.data_;

    other.size_ = 0;
    other.allocator_ = nullptr;
    other.data_ = nullptr;
    return *this;
  }

  template <typename... Targs>
  T& Emplace(usize index, Targs&&... args) {
    COMET_ASSERT(index < size_, "Index out of bounds: ", index, " >= ", size_,
                 "!");
    new (&data_[index]) T{std::forward<Targs>(args)...};
    return data_[index];
  }

  ~FixedSizeArray() {
    if (data_ == nullptr) {
      return;
    }

    Destroy();
  }

  void Destroy() {
    COMET_ASSERT(data_ != nullptr, "Static array has already been destroyed!");

    for (usize i{0}; i < size_; ++i) {
      data_[i].~T();
    }

    allocator_->Deallocate(data_);
    data_ = nullptr;
    size_ = 0;
  }

  T& operator[](usize index) {
    COMET_ASSERT(index < size_, "Index out of bounds! ", index, " >= ", size_,
                 "!");
    return data_[index];
  }

  const T& operator[](usize index) const {
    COMET_ASSERT(index < size_, "Index out of bounds! ", index, " >= ", size_,
                 "!");
    return data_[index];
  }

  usize GetSize() const noexcept { return size_; }

 private:
  usize size_{0};
  memory::AlignedAllocator* allocator_{nullptr};
  T* data_{nullptr};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_FIXED_SIZE_ARRAY_H_