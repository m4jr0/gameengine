// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_ARRAY_H_
#define COMET_COMET_CORE_TYPE_ARRAY_H_

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

#include "comet/core/c_array.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/iterator.h"

namespace comet {
namespace internal {
template <typename T>
class BaseArray {
 public:
  COMET_POPULATE_ITERATOR(ContiguousIterator<T>, ConstContiguousIterator<T>,
                          this->data_, this->size_)

  T& operator[](usize index) {
    COMET_CASSERT(index < this->size_, "Index out of bounds!");
    return this->data_[index];
  }

  const T& operator[](usize index) const {
    COMET_CASSERT(index < this->size_, "Index out of bounds!");
    return this->data_[index];
  }

  bool IsContained(const T& value) const {
    return comet::IsContained(this->data_, this->size_, value);
  }

  usize GetIndex(const T& value) const {
    return comet::GetIndex(this->data_, this->size_, value);
  }

  usize GetSize() const noexcept { return this->size_; };
  T* GetData() noexcept { return this->data_; }
  const T* GetData() const noexcept { return this->data_; }
  bool IsEmpty() const noexcept { return this->size_ == 0; }

 protected:
  BaseArray(usize size = 0, T* data = nullptr) : data_{data}, size_{size} {}

  T* data_{nullptr};
  usize size_{0};
};
}  // namespace internal

template <typename T>
class Array : public internal::BaseArray<T> {
 public:
  Array() = default;

  explicit Array(memory::Allocator* allocator) : allocator_{allocator} {}

  Array(memory::Allocator* allocator, usize size)
      : internal::BaseArray<T>(
            size, size == 0 ? nullptr
                            : static_cast<T*>(allocator->AllocateAligned(
                                  size * sizeof(T), alignof(T)))),
        capacity_{size},
        allocator_{allocator} {
    for (usize i{0}; i < this->size_; ++i) {
      memory::Populate<T>(&this->data_[i]);
    }
  }

  template <typename... Targs>
  Array(memory::Allocator* allocator, Targs&&... args)
      : internal::BaseArray<T>(
            sizeof...(Targs),
            sizeof...(Targs) == 0
                ? nullptr
                : static_cast<T*>(allocator->AllocateAligned(
                      sizeof...(Targs) * sizeof(T), alignof(T)))),
        capacity_{sizeof...(Targs)},
        allocator_{allocator} {
    usize index{0};
    ((memory::Populate<T>(&this->data_[index++], std::forward<T>(args))), ...);
  }

  Array(const Array& other)
      : internal::BaseArray<T>(
            other.size_,
            other.size_ == 0
                ? nullptr
                : static_cast<T*>(other.allocator_->AllocateAligned(
                      other.capacity_ * sizeof(T), alignof(T)))),
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

    if (this->data_ != nullptr) {
      Destroy();
    }

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

    if (this->data_ != nullptr) {
      Destroy();
    }

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

  ~Array() {
    if (this->data_ == nullptr) {
      return;
    }

    Destroy();
  }

  void Destroy() {
    COMET_ASSERT(this->data_ != nullptr,
                 "Static array has already been destroyed!");
    comet::Clear(this->data_, this->size_);
    this->allocator_->Deallocate(this->data_);
    this->data_ = nullptr;
    this->size_ = 0;
  }

  void Reserve(usize new_capacity) {
    this->data_ = comet::Reserve(this->allocator_, this->data_, this->size_,
                                 this->capacity_, new_capacity);
    this->capacity_ = new_capacity;
  }

  void Resize(usize new_size) {
    if (new_size >= this->capacity_) {
      Reserve(new_size);
    }

    for (usize i{this->size_}; i < new_size; ++i) {
      memory::Populate<T>(&this->data_[i], allocator_);
    }

    for (usize i{new_size}; i < this->size_; ++i) {
      this->data_[i].~T();
    }

    this->size_ = new_size;
  }

  void PushBack(const T& value) {
    if (this->size_ == this->capacity_) {
      Reserve(this->capacity_ == 0 ? 1 : this->capacity_ * 2);
    }

    memory::Populate<T>(&this->data_[this->size_++], value);
  }

  void PushBack(T&& value) {
    if (this->size_ == this->capacity_) {
      Reserve(this->capacity_ == 0 ? 1 : this->capacity_ * 2);
    }

    memory::Populate<T>(&this->data_[this->size_++], std::move(value));
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
    if (this->size_ <= this->capacity_) {
      Reserve(this->capacity_ == 0 ? 1 : this->capacity_ * 2);
    }

    memory::Populate<T>(&this->data_[this->size_],
                        std::forward<Targs>(args)...);
    return this->data_[this->size_++];
  }

  ContiguousIterator<T> Remove(ContiguousIterator<T> pos) {
    if (pos + 1 != this->end()) {
      std::move(pos + 1, this->end(), pos);
    }

    --this->size_;
    this->data_[this->size_].~T();
    return pos;
  }

  void Clear() {
    comet::Clear(this->data_, this->size_);
    this->allocator_->Deallocate(this->data_);
    this->data_ = nullptr;
    this->capacity_ = 0;
  }

 private:
  usize capacity_{0};
  memory::Allocator* allocator_{nullptr};
};

template <typename T>
class FixedArray : public internal::BaseArray<T> {
 public:
  FixedArray() = default;

  FixedArray(memory::Allocator* allocator, usize size)
      : internal::BaseArray<T>(
            size, size == 0 ? nullptr
                            : static_cast<T*>(allocator->AllocateAligned(
                                  size * sizeof(T), alignof(T)))),
        allocator_{allocator} {
    for (usize i{0}; i < this->size_; ++i) {
      memory::Populate<T>(&this->data_[i]);
    }
  }

  template <typename... Targs>
  FixedArray(memory::Allocator* allocator, Targs&&... args)
      : internal::BaseArray<T>(
            sizeof...(Targs),
            sizeof...(Targs) == 0
                ? nullptr
                : static_cast<T*>(allocator->AllocateAligned(
                      sizeof...(Targs) * sizeof(T), alignof(T)))),
        allocator_{allocator} {
    usize index{0};
    ((memory::Populate<T>(&this->data_[index++], std::forward<T>(args))), ...);
  }

  FixedArray(const FixedArray& other)
      : internal::BaseArray<T>(
            other.size_,
            other.size_ == 0
                ? nullptr
                : static_cast<T*>(other.allocator_->AllocateAligned(
                      other.size_ * sizeof(T), alignof(T)))),
        allocator_{other.allocator_} {
    if (this->data_ != nullptr) {
      for (usize i{0}; i < this->size_; ++i) {
        memory::Populate<T>(&this->data_[i], other.data_[i]);
      }
    }
  }

  FixedArray(FixedArray&& other) noexcept
      : internal::BaseArray<T>(other.size_, other.data_),
        allocator_{other.allocator_} {
    other.size_ = 0;
    other.data_ = nullptr;
    other.allocator_ = nullptr;
  }

  FixedArray& operator=(const FixedArray& other) {
    if (this == &other) {
      return *this;
    }

    if (this->data_ != nullptr) {
      Destroy();
    }

    this->size_ = other.size_;
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

  FixedArray& operator=(FixedArray&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    if (this->data_ != nullptr) {
      Destroy();
    }

    this->size_ = other.size_;
    this->data_ = other.data_;
    this->allocator_ = other.allocator_;

    other.size_ = 0;
    other.data_ = nullptr;
    other.allocator_ = nullptr;
    return *this;
  }

  ~FixedArray() {
    if (this->data_ == nullptr) {
      return;
    }

    Destroy();
  }

  void Destroy() {
    COMET_ASSERT(this->data_ != nullptr,
                 "Fixed-size array has already been destroyed!");

    for (usize i{0}; i < this->size_; ++i) {
      this->data_[i].~T();
    }

    this->allocator_->Deallocate(this->data_);
    this->data_ = nullptr;
    this->size_ = 0;
  }

 private:
  memory::Allocator* allocator_{nullptr};
};

template <typename T, usize N>
class StaticArray {
 public:
  COMET_POPULATE_ITERATOR(ContiguousIterator<T>, ConstContiguousIterator<T>,
                          this->data_, this->size_)

  constexpr StaticArray() = default;

  template <typename... Targs,
            typename = std::enable_if_t<(sizeof...(Targs) == N)>>
  constexpr StaticArray(Targs... args)
      : data_{static_cast<T>(args)...}, size_{N} {}

  constexpr const T& operator[](usize index) const { return data_[index]; }

  constexpr T& operator[](usize index) { return data_[index]; }

  constexpr bool IsContained(const T& value) const {
    for (usize i{0}; i < size_; ++i) {
      if (data_[i] == value) {
        return true;
      }
    }

    return false;
  }

  constexpr usize GetIndex(const T& value) const {
    for (usize i{0}; i < size_; ++i) {
      if (data_[i] == value) {
        return i;
      }
    }

    return size_;
  }

  constexpr T* GetData() noexcept { return data_; }

  constexpr const T* GetData() const noexcept { return data_; }

  constexpr usize GetSize() const noexcept { return size_; }

  constexpr bool IsEmpty() const noexcept { return size_ > 0; }

 private:
  T data_[N]{};
  usize size_{N};
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

  constexpr bool IsContained(const T& value) const { return false; }

  constexpr usize GetIndex(const T& value) const { return kInvalidIndex; }

  constexpr usize GetSize() const noexcept { return 0; }

  constexpr T* GetData() noexcept { return nullptr; }

  constexpr const T* GetData() const noexcept { return nullptr; }

  constexpr T& operator[](usize) {
    COMET_CASSERT(false, "Index out of bounds!");
    throw std::out_of_range{"index"};
  }

  constexpr const T& operator[](usize) const {
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
StaticArray(First, Rest...)
    -> StaticArray<typename EnforceSame<First, Rest...>::type,
                   1 + sizeof...(Rest)>;
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_ARRAY_H_