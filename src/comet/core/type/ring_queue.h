// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_RING_QUEUE_H_
#define COMET_COMET_CORE_TYPE_RING_QUEUE_H_

#include "comet_precompile.h"
#include "exception.h"

namespace comet {
template <class T>
class ring_queue {
 public:
  explicit ring_queue(uindex);
  ring_queue(const ring_queue&);
  ring_queue(ring_queue&&) noexcept;
  ring_queue& operator=(const ring_queue&);
  ring_queue& operator=(ring_queue&&) noexcept;
  virtual ~ring_queue() = default;

  void push(T&& element);
  void push(const T& element);
  T& front();
  void pop() noexcept;
  void clear() noexcept;
  bool empty() const noexcept;

  uindex capacity() const noexcept;
  uindex size() const noexcept;

 private:
  uindex capacity_{0};
  uindex head_{0};
  uindex size_{0};
  std::vector<T> elements_{};
};

template <class T>
inline ring_queue<T>::ring_queue(uindex capacity)
    : capacity_{capacity}, elements_{std::vector<T>(capacity)} {}

template <class T>
inline ring_queue<T>::ring_queue(const ring_queue<T>& other)
    : capacity_{other.capacity_},
      head_{other.head_},
      size_{other.size_},
      elements_{other.elements_} {}

template <class T>
inline ring_queue<T>::ring_queue(ring_queue<T>&& other) noexcept
    : capacity_{(other.capacity_)},
      head_{other.head_},
      size_{other.size_},
      elements_{std::move(other.elements_)} {
  other.capacity_ = 0;
  other.head_ = 0;
  other.size_ = 0;
  other.elements_.clear();
}

template <class T>
inline ring_queue<T>& ring_queue<T>::operator=(const ring_queue<T>& other) {
  if (this == &other) {
    return *this;
  }

  this->capacity_ = other.capacity_;
  this->head_ = other.head_;
  this->size_ = other.size_;
  this->elements_ = other.elements_;

  return *this;
}

template <class T>
inline ring_queue<T>& ring_queue<T>::operator=(ring_queue<T>&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  this->capacity_ = other.capacity_;
  this->head_ = other.head_;
  this->size_ = other.size_;
  this->elements_ = std::move(other.elements_);
  other.capacity_ = 0;
  other.head_ = 0;
  other.size_ = 0;
  other.elements_.clear();

  return *this;
}

template <class T>
inline void ring_queue<T>::push(T&& element) {
  if (this->size_ == this->capacity_) {
    throw maximum_capacity_reached_error(this->capacity_);
  }

  this->elements_[(this->head_ + this->size_++) % this->capacity_] =
      std::forward<T>(element);
}

template <class T>
inline void ring_queue<T>::push(const T& element) {
  if (this->size_ == this->capacity_) {
    throw maximum_capacity_reached_error(this->capacity_);
  }

  this->elements_[(this->head_ + this->size_++) % this->capacity_] = element;
}

template <class T>
inline T& ring_queue<T>::front() {
  if (this->size_ == 0) {
    throw empty_error();
  }

  return this->elements_[this->head_];
}

template <class T>
inline void ring_queue<T>::pop() noexcept {
  if (this->size_ == 0) {
    return;
  }

  this->head_ = (this->head_ + 1) % this->capacity_;
  --this->size_;
}

template <class T>
inline void ring_queue<T>::clear() noexcept {
  this->head_ = 0;
  this->size_ = 0;
}

template <class T>
inline bool ring_queue<T>::empty() const noexcept {
  return this->size_ == 0;
}

template <class T>
inline uindex ring_queue<T>::capacity() const noexcept {
  return this->capacity_;
}

template <class T>
inline uindex ring_queue<T>::size() const noexcept {
  return this->size_;
}
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_RING_QUEUE_H_
