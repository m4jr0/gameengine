// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_ITERATOR_H_
#define COMET_COMET_CORE_TYPE_ITERATOR_H_

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

  constexpr ContiguousIteratorImpl() noexcept = default;

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

#define COMET_POPULATE_ITERATOR(DataType, DataPtr, SizeValue)                 \
  using Iterator = comet::ContiguousIterator<DataType>;                       \
  using ConstIterator = comet::ConstContiguousIterator<DataType>;             \
                                                                              \
  constexpr Iterator begin() noexcept { return Iterator{DataPtr}; }           \
                                                                              \
  constexpr Iterator end() noexcept { return Iterator{DataPtr + SizeValue}; } \
                                                                              \
  constexpr ConstIterator begin() const noexcept {                            \
    return ConstIterator{DataPtr};                                            \
  }                                                                           \
                                                                              \
  constexpr ConstIterator end() const noexcept {                              \
    return ConstIterator{DataPtr + SizeValue};                                \
  }                                                                           \
                                                                              \
  constexpr ConstIterator cbegin() const noexcept {                           \
    return ConstIterator{DataPtr};                                            \
  }                                                                           \
                                                                              \
  constexpr ConstIterator cend() const noexcept {                             \
    return ConstIterator{DataPtr + SizeValue};                                \
  }                                                                           \
                                                                              \
  constexpr std::reverse_iterator<Iterator> rbegin() noexcept {               \
    return std::reverse_iterator<Iterator>(end());                            \
  }                                                                           \
                                                                              \
  constexpr std::reverse_iterator<Iterator> rend() noexcept {                 \
    return std::reverse_iterator<Iterator>(begin());                          \
  }                                                                           \
                                                                              \
  constexpr std::reverse_iterator<ConstIterator> rbegin() const noexcept {    \
    return std::reverse_iterator<ConstIterator>(end());                       \
  }                                                                           \
                                                                              \
  constexpr std::reverse_iterator<ConstIterator> rend() const noexcept {      \
    return std::reverse_iterator<ConstIterator>(begin());                     \
  }                                                                           \
                                                                              \
  constexpr std::reverse_iterator<ConstIterator> crbegin() const noexcept {   \
    return std::reverse_iterator<ConstIterator>(end());                       \
  }                                                                           \
                                                                              \
  constexpr std::reverse_iterator<ConstIterator> crend() const noexcept {     \
    return std::reverse_iterator<ConstIterator>(begin());                     \
  }

#endif  // COMET_COMET_CORE_TYPE_ITERATOR_H_
