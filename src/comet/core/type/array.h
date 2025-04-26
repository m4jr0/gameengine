// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_ARRAY_H_
#define COMET_COMET_CORE_TYPE_ARRAY_H_

#include <type_traits>
#include <utility>

#include "comet/core/algorithm/algorithm_common.h"
#include "comet/core/c_array.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/iterator.h"
#include "comet/math/math_commons.h"

namespace comet {
namespace internal {
template <typename T>
class BaseArray {
 public:
  COMET_POPULATE_ITERATOR(T, this->data_, this->size_)

  T& operator[](usize index) {
    COMET_CASSERT(index < this->size_, "Index out of bounds!");
    return this->data_[index];
  }

  const T& operator[](usize index) const {
    COMET_CASSERT(index < this->size_, "Index out of bounds!");
    return this->data_[index];
  }

  T& Get(usize index) {
    COMET_CASSERT(index < this->size_, "Index out of bounds!");
    return this->data_[index];
  }

  const T& Get(usize index) const {
    COMET_CASSERT(index < this->size_, "Index out of bounds!");
    return this->data_[index];
  }

  bool IsContained(const T& value) const {
    return comet::IsContained(this->data_, this->size_, value);
  }

  usize GetIndex(const T& value) const {
    return comet::GetIndex(this->data_, this->size_, value);
  }

  T& GetFirst() {
    COMET_CASSERT(this->size_ > 0, "Index out of bounds!");
    return this->data_[0];
  }

  const T& GetFirst() const {
    COMET_CASSERT(this->size_ > 0, "Index out of bounds!");
    return this->data_[0];
  }

  T& GetLast() {
    COMET_CASSERT(this->size_ > 0, "Index out of bounds!");
    return this->data_[size_ - 1];
  }

  const T& GetLast() const {
    COMET_CASSERT(this->size_ > 0, "Index out of bounds!");
    return this->data_[size_ - 1];
  }

  usize GetSize() const noexcept { return this->size_; };
  T* GetData() noexcept { return this->data_; }
  const T* GetData() const noexcept { return this->data_; }
  bool IsEmpty() const noexcept { return this->size_ == 0; }

 protected:
  BaseArray(usize size = 0, T* data = nullptr) : size_{size}, data_{data} {}

  usize size_{0};
  T* data_{nullptr};
};
}  // namespace internal

template <typename T>
class Array : public internal::BaseArray<T> {
 public:
  Array() = default;

  explicit Array(memory::Allocator* allocator)
      : internal::BaseArray<T>{0, nullptr}, allocator_{allocator} {}

  Array(memory::Allocator* allocator, usize capacity)
      : internal::BaseArray<T>{0,
                               capacity == 0
                                   ? nullptr
                                   : static_cast<T*>(allocator->AllocateAligned(
                                         capacity * sizeof(T), alignof(T)))},
        capacity_{capacity},
        allocator_{allocator} {}

  template <typename... Targs>
  Array(memory::Allocator* allocator, Targs&&... args)
      : internal::BaseArray<T>{sizeof...(Targs),
                               sizeof...(Targs) == 0
                                   ? nullptr
                                   : static_cast<T*>(allocator->AllocateAligned(
                                         sizeof...(Targs) * sizeof(T),
                                         alignof(T)))},
        capacity_{sizeof...(Targs)},
        allocator_{allocator} {
    usize index{0};
    ((memory::Populate<T>(&this->data_[index++], std::forward<T>(args))), ...);
  }

  Array(memory::Allocator* allocator, const T* data, usize count)
      : internal::BaseArray<T>(
            count, count == 0 ? nullptr
                              : static_cast<T*>(allocator->AllocateAligned(
                                    count * sizeof(T), alignof(T)))),
        capacity_{count},
        allocator_{allocator} {
    if (this->data_ == nullptr) {
      return;
    }

    for (usize i{0}; i < count; ++i) {
      memory::Populate<T>(&this->data_[i], allocator_, data[i]);
    }
  }

  Array(const Array& other)
      : internal::BaseArray<T>{other.size_,
                               other.size_ == 0
                                   ? nullptr
                                   : static_cast<T*>(
                                         other.allocator_->AllocateAligned(
                                             other.capacity_ * sizeof(T),
                                             alignof(T)))},
        capacity_{other.capacity_},
        allocator_{other.allocator_} {
    if (this->data_ != nullptr) {
      for (usize i{0}; i < this->size_; ++i) {
        memory::Populate<T>(&this->data_[i], other.data_[i]);
      }
    }
  }

  Array(Array&& other) noexcept
      : internal::BaseArray<T>(other.size_, other.data_),
        capacity_{other.capacity_},
        allocator_{other.allocator_} {
    other.size_ = 0;
    other.capacity_ = 0;
    other.data_ = nullptr;
    other.allocator_ = nullptr;
  }

  Array& operator=(const Array& other) {
    if (this == &other) {
      return *this;
    }

    Clear();
    this->size_ = other.size_;
    this->capacity_ = other.capacity_;
    this->allocator_ = other.allocator_;

    if (this->size_ != 0) {
      this->data_ = static_cast<T*>(
          allocator_->AllocateAligned(this->capacity_ * sizeof(T), alignof(T)));
      for (usize i{0}; i < this->size_; ++i) {
        memory::Populate<T>(&this->data_[i], other.data_[i]);
      }
    }

    return *this;
  }

  Array& operator=(Array&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    Clear();
    this->size_ = other.size_;
    this->capacity_ = other.capacity_;
    this->data_ = other.data_;
    this->allocator_ = other.allocator_;

    other.size_ = 0;
    other.capacity_ = 0;
    other.data_ = nullptr;
    other.allocator_ = nullptr;
    return *this;
  }

  ~Array() { Clear(); }

  bool operator==(const Array& other) {
    if (this->size_ != other.size_) {
      return false;
    }

    for (usize i{0}; i < this->size_; ++i) {
      if (!(this->data_[i] == other.data_[i])) {
        return false;
      }
    }

    return true;
  }

  bool operator!=(const Array& other) { return !operator=(other); }

  void Reserve(usize new_capacity) {
    this->data_ = comet::Reserve(this->allocator_, this->data_, this->size_,
                                 this->capacity_, new_capacity);
    this->capacity_ = new_capacity;
  }

  void Resize(usize new_size) {
    if (new_size > this->capacity_) {
      Reserve(new_size);
    }

    if constexpr (std::is_trivially_constructible_v<T>) {
      if (new_size > this->size_) {
        memory::ClearMemory(this->data_ + this->size_,
                            (new_size - this->size_) * sizeof(T));
      }
    } else {
      for (usize i{this->size_}; i < new_size; ++i) {
        memory::Populate<T>(&this->data_[i], allocator_);
      }

      for (usize i{new_size}; i < this->size_; ++i) {
        this->data_[i].~T();
      }
    }

    this->size_ = new_size;
  }

  template <typename U>
  void PushBack(U&& value) {
    if (this->size_ == this->capacity_) {
      Reserve(this->capacity_ == 0 ? 1 : this->capacity_ * 2);
    }

    memory::Populate<T>(&this->data_[this->size_++],
                        static_cast<T>(std::forward<U>(value)));
  }

  template <typename... Targs>
  T& Emplace(usize index, Targs&&... args) {
    COMET_ASSERT(index < this->size_, "Index out of bounds: ", index,
                 " >= ", this->size_, "!");
    memory::Populate<T>(&this->data_[index], std::forward<Targs>(args)...);
    return this->data_[index];
  }

  template <typename... Targs>
  T& EmplaceBack(Targs&&... args) {
    if (this->size_ == this->capacity_) {
      Reserve(this->capacity_ == 0 ? 1 : this->capacity_ * 2);
    }

    memory::Populate<T>(&this->data_[this->size_],
                        std::forward<Targs>(args)...);
    return this->data_[this->size_++];
  }

  ContiguousIterator<T> RemoveFromPos(ContiguousIterator<T> pos) {
    if (pos + 1 != this->end()) {
      std::move(pos + 1, this->end(), pos);
    }

    --this->size_;
    this->data_[this->size_].~T();
    return pos;
  }

  ContiguousIterator<T> RemoveFromValue(const T& value) {
    auto it{this->begin()};

    for (; it != this->end(); ++it) {
      if (*it == value) {
        break;
      }
    }

    if (it == this->end()) {
      return this->end();
    }

    return RemoveFromPos(it);
  }

  ContiguousIterator<T> RemoveFromIndex(usize index) {
    COMET_ASSERT(index < this->size_, "Index out of bounds: ", index,
                 " >= ", this->size_, "!");
    return RemoveFromPos(this->begin() + index);
  }

  void PushFromRange(const T* src, usize src_size, usize count = kInvalidIndex,
                     usize dst_offset = kInvalidIndex, usize src_offset = 0) {
    COMET_ASSERT(src != nullptr, "Source array is null!");

    if (count == 0) {
      return;
    }

    if (count == kInvalidIndex) {
      count = src_size;
    }

    if (dst_offset == kInvalidIndex) {
      dst_offset = this->size_;
    }

    auto new_size{math::Max(this->size_, dst_offset + count)};
    Reserve(new_size);
    comet::Copy(this->data_, this->capacity_, src, src_size, count, dst_offset,
                src_offset);
    this->size_ = new_size;
  }

  template <typename TArray>
  void PushFromRange(const TArray& src, usize count = kInvalidIndex,
                     usize dst_offset = kInvalidIndex, usize src_offset = 0) {
    static_assert(std::is_same_v<decltype(src.GetData()), const T*> &&
                      std::is_same_v<decltype(src.GetSize()), usize>,
                  "TArray must have GetData() and GetSize() methods!");
    PushFromRange(src.GetData(), src.GetSize(), count, dst_offset, src_offset);
  }

  void Clear() {
    if (this->data_ != nullptr) {
      comet::Clear(this->data_, this->size_);
      this->allocator_->Deallocate(this->data_);
      this->data_ = nullptr;
      this->size_ = 0;
    }

    this->capacity_ = 0;
  }

  usize GetCapacity() const noexcept { return this->capacity_; };

 private:
  usize capacity_{0};
  memory::Allocator* allocator_{nullptr};
};

template <typename T, usize N>
class StaticArray {
 public:
  COMET_POPULATE_ITERATOR(T, this->data_, N)

  constexpr StaticArray() = default;

  template <typename... Targs,
            typename = std::enable_if_t<(sizeof...(Targs) == N)>>
  constexpr StaticArray(Targs... args) : data_{static_cast<T>(args)...} {}

  constexpr T& operator[](usize index) { return data_[index]; }

  constexpr const T& operator[](usize index) const { return data_[index]; }

  constexpr T& Get(usize index) { return data_[index]; }

  constexpr const T& Get(usize index) const { return data_[index]; }

  constexpr bool operator==(const StaticArray& other) {
    for (usize i{0}; i < N; ++i) {
      if (!(this->data_[i] == other.data_[i])) {
        return false;
      }
    }

    return true;
  }

  constexpr bool operator!=(const StaticArray& other) {
    return !operator==(other);
  }

  constexpr bool IsContained(const T& value) const {
    for (usize i{0}; i < N; ++i) {
      if (data_[i] == value) {
        return true;
      }
    }

    return false;
  }

  constexpr usize GetIndex(const T& value) const {
    for (usize i{0}; i < N; ++i) {
      if (data_[i] == value) {
        return i;
      }
    }

    return kInvalidIndex;
  }

  constexpr T* GetData() noexcept { return data_; }

  constexpr const T* GetData() const noexcept { return data_; }

  constexpr usize GetSize() const noexcept { return N; }

  constexpr bool IsEmpty() const noexcept { return N == 0; }

  T& GetFirst() { return this->data_[0]; }

  const T& GetFirst() const { return this->data_[0]; }

  T& GetLast() { return this->data_[N - 1]; }

  const T& GetLast() const { return this->data_[N - 1]; }

 private:
  T data_[N]{};
};

template <typename T>
class StaticArray<T, 0> {
 public:
  static_assert(std::is_object_v<T>, "T must be an object type!");

  constexpr T* begin() noexcept { return nullptr; }

  constexpr T* end() noexcept { return nullptr; }

  constexpr const T* begin() const noexcept { return nullptr; }

  constexpr const T* end() const noexcept { return nullptr; }

  constexpr const T* cbegin() const noexcept { return nullptr; }

  constexpr const T* cend() const noexcept { return nullptr; }

  constexpr std::reverse_iterator<T> rbegin() noexcept {
    return std::reverse_iterator<T>(end());
  }

  constexpr std::reverse_iterator<T> rend() noexcept {
    return std::reverse_iterator<T>(begin());
  }

  constexpr std::reverse_iterator<const T*> rbegin() const noexcept {
    return std::reverse_iterator<const T*>(end());
  }

  constexpr std::reverse_iterator<const T*> rend() const noexcept {
    return std::reverse_iterator<const T*>(begin());
  }

  constexpr std::reverse_iterator<const T*> crbegin() const noexcept {
    return std::reverse_iterator<const T*>(end());
  }

  constexpr std::reverse_iterator<const T*> crend() const noexcept {
    return std::reverse_iterator<const T*>(begin());
  }

  constexpr StaticArray() {}

  constexpr bool operator==(const StaticArray&) { return true; }

  constexpr bool operator!=(const StaticArray&) { return false; }

  constexpr bool IsContained(const T& value) const { return false; }

  constexpr usize GetIndex(const T& value) const { return kInvalidIndex; }

  constexpr usize GetSize() const noexcept { return 0; }

  constexpr bool IsEmpty() const noexcept { return true; }

  constexpr T* GetData() noexcept { return nullptr; }

  constexpr const T* GetData() const noexcept { return nullptr; }

  T& GetFirst() {
    COMET_CASSERT(false, "Index out of bounds!");
    return this->data_[0];
  }

  const T& GetFirst() const {
    COMET_CASSERT(false, "Index out of bounds!");
    return this->data_[0];
  }

  T& GetLast() {
    COMET_CASSERT(false, "Index out of bounds!");
    return this->data_[0];
  }

  const T& GetLast() const {
    COMET_CASSERT(false, "Index out of bounds!");
    return this->data_[0];
  }

  constexpr T& operator[](usize) {
    COMET_CASSERT(false, "Index out of bounds!");
    throw std::out_of_range{"index"};
  }

  constexpr const T& operator[](usize) const {
    COMET_CASSERT(false, "Index out of bounds!");
    throw std::out_of_range{"index"};
  }

  constexpr T& Get(usize index) {
    COMET_CASSERT(false, "Index out of bounds!");
    throw std::out_of_range{"index"};
  }

  constexpr const T& Get(usize index) const {
    COMET_CASSERT(false, "Index out of bounds!");
    throw std::out_of_range{"index"};
  }
};

template <typename T, typename... Rest>
struct EnforceSame {
  static_assert(std::conjunction_v<std::is_same<T, Rest>...>,
                "All elements must have the same type!");
  using type = T;
};

template <typename First, typename... Rest>
StaticArray(First,
            Rest...) -> StaticArray<typename EnforceSame<First, Rest...>::type,
                                    1 + sizeof...(Rest)>;
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_ARRAY_H_