// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_ARRAY_H_
#define COMET_COMET_CORE_TYPE_ARRAY_H_

// External. ///////////////////////////////////////////////////////////////////
#include <stdexcept>
#include <type_traits>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_array.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/iterator.h"
#include "comet/math/math_scalar.h"

namespace comet {
namespace internal {
template <typename T>
class BaseArray {
 public:
  COMET_POPULATE_ITERATOR(T, this->data_, this->size_)

  T& operator[](usize index) {
    COMET_CASSERT(index < this->size_, "index out of bounds");
    return this->data_[index];
  }

  const T& operator[](usize index) const {
    COMET_CASSERT(index < this->size_, "index out of bounds");
    return this->data_[index];
  }

  T& Get(usize index) {
    COMET_CASSERT(index < this->size_, "index out of bounds");
    return this->data_[index];
  }

  const T& Get(usize index) const {
    COMET_CASSERT(index < this->size_, "index out of bounds");
    return this->data_[index];
  }

  template <typename U>
  T& Set(usize index, U&& value) {
    auto& slot{this->operator[](index)};
    slot = std::forward<U>(value);
    return slot;
  }

  bool IsContained(const T& value) const {
    return comet::IsContained(this->data_, this->size_, value);
  }

  usize GetIndex(const T& value) const {
    return comet::GetIndex(this->data_, this->size_, value);
  }

  T& GetFirst() {
    COMET_CASSERT(this->size_ > 0, "index out of bounds");
    return this->data_[0];
  }

  const T& GetFirst() const {
    COMET_CASSERT(this->size_ > 0, "index out of bounds");
    return this->data_[0];
  }

  T& GetLast() {
    COMET_CASSERT(this->size_ > 0, "index out of bounds");
    return this->data_[this->size_ - 1];
  }

  const T& GetLast() const {
    COMET_CASSERT(this->size_ > 0, "index out of bounds");
    return this->data_[this->size_ - 1];
  }

  usize GetSize() const noexcept { return this->size_; }
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
  struct WithCapacityTag {};
  struct FromDataTag {};

 public:
  static Array WithCapacity(memory::Allocator* allocator, usize capacity) {
    return Array{allocator, WithCapacityTag{}, capacity};
  }

  static Array FromData(memory::Allocator* allocator, const T* data,
                        usize count) {
    return Array{allocator, FromDataTag{}, data, count};
  }

  Array() = default;

  explicit Array(memory::Allocator* allocator)
      : internal::BaseArray<T>{0, nullptr}, allocator_{allocator} {}

  template <typename... Targs,
            typename = std::enable_if_t<(sizeof...(Targs) > 1)>>
  Array(memory::Allocator* allocator, Targs&&... args)
      : internal::BaseArray<T>{0, nullptr},
        capacity_{sizeof...(Targs)},
        allocator_{allocator} {
    COMET_ASSERT(allocator_ != nullptr, "Array::Array", "allocator is null");

    this->data_ = static_cast<T*>(
        allocator_->AllocateAligned(capacity_ * sizeof(T), alignof(T)));

    usize index{0};

    ((memory::Populate<T>(&this->data_[index++], std::forward<Targs>(args))),
     ...);

    this->size_ = capacity_;
  }

  Array(const Array& other)
      : internal::BaseArray<T>{0, nullptr},
        capacity_{other.capacity_},
        allocator_{other.allocator_} {
    COMET_ASSERT(capacity_ == 0 || allocator_ != nullptr, "Array::Array",
                 "source allocator is null");

    if (capacity_ != 0) {
      this->data_ = static_cast<T*>(
          allocator_->AllocateAligned(capacity_ * sizeof(T), alignof(T)));

      if constexpr (std::is_trivially_copyable_v<T>) {
        memory::CopyMemory(this->data_, other.data_, other.size_ * sizeof(T));
      } else {
        for (usize i{0}; i < other.size_; ++i) {
          memory::Populate<T>(&this->data_[i], other.data_[i]);
        }
      }
    }

    this->size_ = other.size_;
  }

  Array(Array&& other) noexcept
      : internal::BaseArray<T>{other.size_, other.data_},
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

    Release();

    capacity_ = other.capacity_;
    allocator_ = other.allocator_;

    COMET_ASSERT(capacity_ == 0 || allocator_ != nullptr,
                 "Array::operator=", "source allocator is null");

    if (capacity_ != 0) {
      this->data_ = static_cast<T*>(
          allocator_->AllocateAligned(capacity_ * sizeof(T), alignof(T)));

      if constexpr (std::is_trivially_copyable_v<T>) {
        memory::CopyMemory(this->data_, other.data_, other.size_ * sizeof(T));
      } else {
        for (usize i{0}; i < other.size_; ++i) {
          memory::Populate<T>(&this->data_[i], other.data_[i]);
        }
      }
    }

    this->size_ = other.size_;
    return *this;
  }

  Array& operator=(Array&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    Release();

    this->size_ = other.size_;
    capacity_ = other.capacity_;
    this->data_ = other.data_;
    allocator_ = other.allocator_;

    other.size_ = 0;
    other.capacity_ = 0;
    other.data_ = nullptr;
    other.allocator_ = nullptr;

    return *this;
  }

  ~Array() { Release(); }

  bool operator==(const Array& other) const {
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

  bool operator!=(const Array& other) const { return !operator==(other); }

  void Release() {
    if (this->data_ != nullptr) {
      COMET_ASSERT(allocator_ != nullptr, "Array::Release",
                   "allocator is null while data is not null");
      comet::Clear(this->data_, this->size_);
      allocator_->Deallocate(this->data_);
    }

    allocator_ = nullptr;
    this->data_ = nullptr;
    this->size_ = 0;
    capacity_ = 0;
  }

  void Reserve(usize new_capacity) {
    if (new_capacity <= capacity_) {
      return;
    }

    COMET_ASSERT(allocator_ != nullptr, "Array::Reserve", "allocator is null");

    this->data_ = comet::Reserve(allocator_, this->data_, this->size_,
                                 capacity_, new_capacity);
    capacity_ = new_capacity;
  }

  void TrimCapacity() {
    if (this->size_ == capacity_) {
      return;
    }

    COMET_ASSERT(allocator_ != nullptr, "Array::TrimCapacity",
                 "allocator is null");

    this->data_ =
        comet::TrimCapacity(allocator_, this->data_, this->size_, capacity_);
    capacity_ = this->size_;
  }

  void Resize(usize new_size) {
    if (new_size > capacity_) {
      Reserve(new_size);
    }

    if (new_size > this->size_) {
      if constexpr (std::is_trivially_constructible_v<T>) {
        memory::ClearMemory(this->data_ + this->size_,
                            (new_size - this->size_) * sizeof(T));
      } else {
        for (usize i{this->size_}; i < new_size; ++i) {
          memory::Populate<T>(&this->data_[i], allocator_);
        }
      }
    } else if (new_size < this->size_) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        for (usize i{new_size}; i < this->size_; ++i) {
          this->data_[i].~T();
        }
      }
    }

    this->size_ = new_size;
  }

  template <typename U>
  void PushLast(U&& value) {
    if (this->size_ == capacity_) {
      Reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }

    memory::Populate<T>(&this->data_[this->size_++],
                        static_cast<T>(std::forward<U>(value)));
  }

  template <typename... Targs>
  T& Emplace(usize index, Targs&&... args) {
    COMET_ASSERT(index < this->size_, "Array::Emplace", "index out of bounds",
                 "index", index, "size", this->size_);

    if constexpr (!std::is_trivially_destructible_v<T>) {
      this->data_[index].~T();
    }

    memory::Populate<T>(&this->data_[index], std::forward<Targs>(args)...);
    return this->data_[index];
  }

  template <typename... Targs>
  T& EmplaceLast(Targs&&... args) {
    if (this->size_ == capacity_) {
      Reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }

    memory::Populate<T>(&this->data_[this->size_],
                        std::forward<Targs>(args)...);
    return this->data_[this->size_++];
  }

  void PopLast() {
    COMET_ASSERT(this->size_ > 0, "Array::PopLast", "array is empty");

    --this->size_;

    if constexpr (!std::is_trivially_destructible_v<T>) {
      this->data_[this->size_].~T();
    }
  }

  T TakeLast() {
    COMET_ASSERT(this->size_ > 0, "Array::TakeLast", "array is empty");

    const auto index{this->size_ - 1};
    T value{std::move(this->data_[index])};

    --this->size_;

    if constexpr (!std::is_trivially_destructible_v<T>) {
      this->data_[index].~T();
    }

    return value;
  }

  ContiguousIterator<T> RemoveFromPos(ContiguousIterator<T> pos) {
    if (pos + 1 != this->end()) {
      std::move(pos + 1, this->end(), pos);
    }

    --this->size_;

    if constexpr (!std::is_trivially_destructible_v<T>) {
      this->data_[this->size_].~T();
    }

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
    COMET_ASSERT(index < this->size_, "Array::RemoveFromIndex",
                 "index out of bounds", "index", index, "size", this->size_);

    return RemoveFromPos(this->begin() + index);
  }

  void PushFromRange(const T* src, usize src_size, usize count = kInvalidIndex,
                     usize dst_offset = kInvalidIndex, usize src_offset = 0) {
    COMET_ASSERT(src != nullptr, "Array::PushFromRange",
                 "source array is null");
    COMET_ASSERT(src_offset <= src_size, "Array::PushFromRange",
                 "source offset exceeds bounds", "src_offset", src_offset,
                 "src_size", src_size);

    if (count == kInvalidIndex) {
      count = src_size - src_offset;
    }

    if (count == 0) {
      return;
    }

    COMET_ASSERT(src_offset + count <= src_size, "Array::PushFromRange",
                 "source range exceeds bounds", "src_offset", src_offset,
                 "count", count, "src_size", src_size);

    if (dst_offset == kInvalidIndex) {
      dst_offset = this->size_;
    }

    const auto* old_data{this->data_};
    const auto old_size{this->size_};
    const auto new_size{math::Max(old_size, dst_offset + count)};

    const auto* src_begin{src + src_offset};
    const auto* src_end{src_begin + count};

    [[maybe_unused]] const auto is_self_range{old_data != nullptr &&
                                              src_begin < old_data + old_size &&
                                              old_data < src_end};

    COMET_ASSERT(!is_self_range || new_size <= capacity_,
                 "Array::PushFromRange",
                 "self-range PushFromRange cannot grow because source would be "
                 "invalidated");

    Reserve(new_size);

    if constexpr (std::is_trivially_copyable_v<T>) {
      memory::CopyOrMoveMemory(this->data_ + dst_offset, src_begin,
                               count * sizeof(T));
    } else {
      const bool copy_backward{is_self_range &&
                               (old_data + dst_offset > src_begin)};

      if (copy_backward) {
        for (usize n{count}; n > 0; --n) {
          const auto i{n - 1};
          const auto dst_index{dst_offset + i};
          const auto src_index{src_offset + i};

          if (dst_index < old_size) {
            this->data_[dst_index] = src[src_index];
          } else {
            memory::Populate<T>(&this->data_[dst_index], src[src_index]);
          }
        }
      } else {
        for (usize i{0}; i < count; ++i) {
          const auto dst_index{dst_offset + i};
          const auto src_index{src_offset + i};

          if (dst_index < old_size) {
            this->data_[dst_index] = src[src_index];
          } else {
            memory::Populate<T>(&this->data_[dst_index], src[src_index]);
          }
        }
      }
    }

    this->size_ = new_size;
  }

  template <typename TArray,
            typename = std::enable_if_t<!std::is_array_v<TArray>>>
  void PushFromRange(const TArray& src, usize count = kInvalidIndex,
                     usize dst_offset = kInvalidIndex, usize src_offset = 0) {
    static_assert(std::is_convertible_v<decltype(src.GetData()), const T*> &&
                      std::is_same_v<decltype(src.GetSize()), usize>,
                  "TArray must have GetData() and GetSize() methods");

    PushFromRange(src.GetData(), src.GetSize(), count, dst_offset, src_offset);
  }

  template <usize N>
  void PushFromRange(const T (&src)[N], usize count = kInvalidIndex,
                     usize dst_offset = kInvalidIndex, usize src_offset = 0) {
    PushFromRange(src, N, count, dst_offset, src_offset);
  }

  void Clear() {
    if (this->data_ != nullptr) {
      comet::Clear(this->data_, this->size_);
    }

    this->size_ = 0;
  }

  usize GetCapacity() const noexcept { return capacity_; }
  memory::Allocator* GetAllocator() noexcept { return allocator_; }
  const memory::Allocator* GetAllocator() const noexcept { return allocator_; }

 private:
  Array(memory::Allocator* allocator, WithCapacityTag, usize capacity)
      : internal::BaseArray<T>{}, capacity_{capacity}, allocator_{allocator} {
    COMET_ASSERT(capacity == 0 || allocator != nullptr, "Array::WithCapacity",
                 "allocator is null");

    if (capacity != 0) {
      this->data_ = static_cast<T*>(
          allocator->AllocateAligned(capacity * sizeof(T), alignof(T)));
    }
  }

  Array(memory::Allocator* allocator, FromDataTag, const T* data, usize count)
      : internal::BaseArray<T>{0, nullptr},
        capacity_{count},
        allocator_{allocator} {
    COMET_ASSERT(count == 0 || allocator_ != nullptr, "Array::FromData",
                 "allocator is null");
    COMET_ASSERT(count == 0 || data != nullptr, "Array::FromData",
                 "source data is null");

    if (count != 0) {
      this->data_ = static_cast<T*>(
          allocator_->AllocateAligned(count * sizeof(T), alignof(T)));

      for (usize i{0}; i < count; ++i) {
        memory::Populate<T>(&this->data_[i], data[i]);
      }
    }

    this->size_ = count;
  }

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

  template <typename U>
  constexpr T& Set(usize index, U&& value) {
    data_[index] = std::forward<U>(value);
    return data_[index];
  }

  constexpr bool operator==(const StaticArray& other) const {
    for (usize i{0}; i < N; ++i) {
      if (!(data_[i] == other.data_[i])) {
        return false;
      }
    }

    return true;
  }

  constexpr bool operator!=(const StaticArray& other) const {
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

  T& GetFirst() { return data_[0]; }

  const T& GetFirst() const { return data_[0]; }

  T& GetLast() { return data_[N - 1]; }

  const T& GetLast() const { return data_[N - 1]; }

 private:
  T data_[N]{};
};

template <typename T>
class StaticArray<T, 0> {
 public:
  static_assert(std::is_object_v<T>, "T must be an object type");

  constexpr T* begin() noexcept { return nullptr; }

  constexpr T* end() noexcept { return nullptr; }

  constexpr const T* begin() const noexcept { return nullptr; }

  constexpr const T* end() const noexcept { return nullptr; }

  constexpr const T* cbegin() const noexcept { return nullptr; }

  constexpr const T* cend() const noexcept { return nullptr; }

  constexpr std::reverse_iterator<T*> rbegin() noexcept {
    return std::reverse_iterator<T*>(end());
  }

  constexpr std::reverse_iterator<T*> rend() noexcept {
    return std::reverse_iterator<T*>(begin());
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

  constexpr bool operator==(const StaticArray&) const { return true; }

  constexpr bool operator!=(const StaticArray&) const { return false; }

  constexpr bool IsContained(const T&) const { return false; }

  constexpr usize GetIndex(const T&) const { return kInvalidIndex; }

  constexpr usize GetSize() const noexcept { return 0; }

  constexpr bool IsEmpty() const noexcept { return true; }

  constexpr T* GetData() noexcept { return nullptr; }

  constexpr const T* GetData() const noexcept { return nullptr; }

  T& GetFirst() {
    COMET_CASSERT(false, "index out of bounds");
    throw std::out_of_range{};
  }

  const T& GetFirst() const {
    COMET_CASSERT(false, "index out of bounds");
    throw std::out_of_range{};
  }

  T& GetLast() {
    COMET_CASSERT(false, "index out of bounds");
    throw std::out_of_range{};
  }

  const T& GetLast() const {
    COMET_CASSERT(false, "index out of bounds");
    throw std::out_of_range{};
  }

  constexpr T& operator[](usize) {
    COMET_CASSERT(false, "index out of bounds");
    throw std::out_of_range{"index"};
  }

  constexpr const T& operator[](usize) const {
    COMET_CASSERT(false, "index out of bounds");
    throw std::out_of_range{"index"};
  }

  constexpr T& Get(usize) {
    COMET_CASSERT(false, "index out of bounds");
    throw std::out_of_range{"index"};
  }

  constexpr const T& Get(usize) const {
    COMET_CASSERT(false, "index out of bounds");
    throw std::out_of_range{"index"};
  }

  template <typename U>
  constexpr T& Set(usize, U&&) {
    COMET_CASSERT(false, "index out of bounds");
    throw std::out_of_range{"index"};
  }
};

template <typename T, typename... Rest>
struct EnforceSame {
  static_assert(std::conjunction_v<std::is_same<T, Rest>...>,
                "all elements must have the same type");
  using type = T;
};

template <typename First, typename... Rest>
StaticArray(First, Rest...)
    -> StaticArray<typename EnforceSame<First, Rest...>::type,
                   1 + sizeof...(Rest)>;
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_ARRAY_H_