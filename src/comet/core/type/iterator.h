// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_ITERATOR_H_
#define COMET_COMET_CORE_TYPE_ITERATOR_H_

#include <cstddef>
#include <iterator>
#include <type_traits>

#include "comet/core/essentials.h"

namespace comet {
template <typename T, bool IsConst>
class ContiguousIteratorImpl {
 public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using pointer = std::conditional_t<IsConst, const T*, T*>;
  using reference = std::conditional_t<IsConst, const T&, T&>;

  constexpr ContiguousIteratorImpl(pointer ptr) noexcept : ptr_{ptr} {}

  constexpr reference operator*() const noexcept { return *ptr_; }
  constexpr pointer operator->() const noexcept { return ptr_; }

  constexpr bool operator==(
      const ContiguousIteratorImpl& other) const noexcept {
    return ptr_ == other.ptr_;
  }

  constexpr bool operator!=(
      const ContiguousIteratorImpl& other) const noexcept {
    return ptr_ != other.ptr_;
  }

  constexpr ContiguousIteratorImpl& operator++() noexcept {
    ++ptr_;
    return *this;
  }

  constexpr ContiguousIteratorImpl operator++(int) noexcept {
    auto tmp{*this};
    ++(*this);
    return tmp;
  }

  constexpr ContiguousIteratorImpl& operator--() noexcept {
    --ptr_;
    return *this;
  }

  constexpr ContiguousIteratorImpl operator--(int) noexcept {
    auto tmp{*this};
    --(*this);
    return tmp;
  }

  constexpr ContiguousIteratorImpl& operator+=(
      difference_type offset) noexcept {
    ptr_ += offset;
    return *this;
  }

  constexpr ContiguousIteratorImpl& operator-=(
      difference_type offset) noexcept {
    ptr_ -= offset;
    return *this;
  }

  constexpr ContiguousIteratorImpl operator+(
      difference_type offset) const noexcept {
    return ContiguousIteratorImpl{ptr_ + offset};
  }

  constexpr ContiguousIteratorImpl operator-(
      difference_type offset) const noexcept {
    return ContiguousIteratorImpl{ptr_ - offset};
  }

  constexpr difference_type operator-(
      const ContiguousIteratorImpl& other) const noexcept {
    return ptr_ - other.ptr_;
  }

  constexpr bool operator<(const ContiguousIteratorImpl& other) const noexcept {
    return ptr_ < other.ptr_;
  }

  constexpr bool operator<=(
      const ContiguousIteratorImpl& other) const noexcept {
    return ptr_ <= other.ptr_;
  }

  constexpr bool operator>(const ContiguousIteratorImpl& other) const noexcept {
    return ptr_ > other.ptr_;
  }

  constexpr bool operator>=(
      const ContiguousIteratorImpl& other) const noexcept {
    return ptr_ >= other.ptr_;
  }

  constexpr reference operator[](difference_type index) const noexcept {
    return *(ptr_ + index);
  }

 private:
  pointer ptr_{nullptr};
};

template <typename T>
using ContiguousIterator = ContiguousIteratorImpl<T, false>;

template <typename T>
using ConstContiguousIterator = ContiguousIteratorImpl<T, true>;
}  // namespace comet

#define COMET_POPULATE_ITERATOR(IteratorType, ConstIteratorType, DataPointer,  \
                                SizeValue)                                     \
  constexpr IteratorType begin() noexcept {                                    \
    return IteratorType{DataPointer};                                          \
  }                                                                            \
                                                                               \
  constexpr IteratorType end() noexcept {                                      \
    return IteratorType{DataPointer + SizeValue};                              \
  }                                                                            \
                                                                               \
  constexpr ConstIteratorType begin() const noexcept {                         \
    return ConstIteratorType{DataPointer};                                     \
  }                                                                            \
                                                                               \
  constexpr ConstIteratorType end() const noexcept {                           \
    return ConstIteratorType{DataPointer + SizeValue};                         \
  }                                                                            \
                                                                               \
  constexpr ConstIteratorType cbegin() const noexcept {                        \
    return ConstIteratorType{DataPointer};                                     \
  }                                                                            \
                                                                               \
  constexpr ConstIteratorType cend() const noexcept {                          \
    return ConstIteratorType{DataPointer + SizeValue};                         \
  }                                                                            \
                                                                               \
  constexpr std::reverse_iterator<IteratorType> rbegin() noexcept {            \
    return std::reverse_iterator<IteratorType>(end());                         \
  }                                                                            \
                                                                               \
  constexpr std::reverse_iterator<IteratorType> rend() noexcept {              \
    return std::reverse_iterator<IteratorType>(begin());                       \
  }                                                                            \
                                                                               \
  constexpr std::reverse_iterator<ConstIteratorType> rbegin() const noexcept { \
    return std::reverse_iterator<ConstIteratorType>(end());                    \
  }                                                                            \
                                                                               \
  constexpr std::reverse_iterator<ConstIteratorType> rend() const noexcept {   \
    return std::reverse_iterator<ConstIteratorType>(begin());                  \
  }                                                                            \
                                                                               \
  constexpr std::reverse_iterator<ConstIteratorType> crbegin()                 \
      const noexcept {                                                         \
    return std::reverse_iterator<ConstIteratorType>(end());                    \
  }                                                                            \
                                                                               \
  constexpr std::reverse_iterator<ConstIteratorType> crend() const noexcept {  \
    return std::reverse_iterator<ConstIteratorType>(begin());                  \
  }

#endif  // COMET_COMET_CORE_TYPE_ITERATOR_H_
