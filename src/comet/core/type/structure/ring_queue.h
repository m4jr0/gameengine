// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_STRUCTURE_RING_QUEUE_H_
#define COMET_COMET_CORE_TYPE_STRUCTURE_RING_QUEUE_H_

#include "comet_precompile.h"

#include "exception.h"

namespace comet {
// Abstract ring queue with a fixed capacity given at its instanciation.
// The public member naming rules follow the STL containers to ease its use.
template <class T>
class abstract_ring_queue {
 public:
  explicit abstract_ring_queue(uindex capacity);
  abstract_ring_queue(const abstract_ring_queue&);
  abstract_ring_queue(abstract_ring_queue&&) noexcept;
  abstract_ring_queue& operator=(const abstract_ring_queue&);
  abstract_ring_queue& operator=(abstract_ring_queue&&) noexcept;

  // By default, there is not empty() member to prevent from using it in the
  // thread-safe implementation and therefore having TOCTTOU bugs.
  virtual void push(T&& element) = 0;
  virtual T& front() = 0;
  virtual void pop() noexcept = 0;
  virtual void clear() noexcept = 0;

  virtual uindex capacity() const noexcept;
  virtual uindex size() const noexcept = 0;

 protected:
  uindex capacity_{0};
};

template <class T>
inline abstract_ring_queue<T>::abstract_ring_queue(uindex capacity)
    : capacity_{capacity} {}

template <class T>
inline abstract_ring_queue<T>::abstract_ring_queue(
    const abstract_ring_queue<T>& other)
    : capacity_{other.capacity_} {}

template <class T>
inline abstract_ring_queue<T>::abstract_ring_queue(
    abstract_ring_queue<T>&& other) noexcept
    : capacity_{other.capacity_} {
  other.capacity_ = 0;
}

template <class T>
inline abstract_ring_queue<T>& abstract_ring_queue<T>::operator=(
    const abstract_ring_queue<T>& other) {
  if (this == &other) {
    return *this;
  }

  this->capacity_ = other.capacity;
  return *this;
}

template <class T>
inline abstract_ring_queue<T>& abstract_ring_queue<T>::operator=(
    abstract_ring_queue<T>&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  this->capacity_ = capacity_;
  other.capacity_ = 0;
  return *this;
}

template <class T>
inline uindex abstract_ring_queue<T>::capacity() const noexcept {
  return this->capacity_;
}

// Non thread-safe ring queue with a fixed capacity given at its instanciation.
// The public member naming rules follow the STL containers to ease its use.
template <class T>
class ring_queue : public abstract_ring_queue<T> {
 public:
  explicit ring_queue(uindex);
  ring_queue(const ring_queue&);
  ring_queue(ring_queue&&) noexcept;
  ring_queue& operator=(const ring_queue&);
  ring_queue& operator=(ring_queue&&) noexcept;
  virtual ~ring_queue() = default;

  void push(T&& element) override;
  T& front() override;
  void pop() noexcept override;
  void clear() noexcept override;
  bool empty() const noexcept;

  uindex size() const noexcept override;

 private:
  uindex head_{0};
  uindex size_{0};
  std::vector<T> elements_{};
};

template <class T>
inline ring_queue<T>::ring_queue(uindex capacity)
    : abstract_ring_queue<T>{capacity}, elements_{std::vector<T>(capacity)} {}

template <class T>
inline ring_queue<T>::ring_queue(const ring_queue<T>& other)
    : abstract_ring_queue<T>{other},
      head_{other.head_},
      size_{other.size_},
      elements_{other.elements_} {}

template <class T>
inline ring_queue<T>::ring_queue(ring_queue<T>&& other) noexcept
    : abstract_ring_queue<T>{std::move(other)},
      head_{other.head_},
      size_{other.size_},
      elements_{std::move(other.elements_)} {
  other.head_ = 0;
  other.size_ = 0;
  other.elements_.clear();
}

template <class T>
inline ring_queue<T>& ring_queue<T>::operator=(const ring_queue<T>& other) {
  if (this == &other) {
    return *this;
  }

  abstract_ring_queue<T>::operator=(other);
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

  abstract_ring_queue<T>::operator=(std::move(other));
  this->head_ = other.head_;
  this->size_ = other.size_;
  this->elements_ = std::move(other.elements_);
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
inline uindex ring_queue<T>::size() const noexcept {
  return this->size_;
}

// Thread-safe ring queue with a fixed capacity given at its instanciation.
// The public member naming rules follow the STL containers to ease its use.
template <class T>
class concurrent_ring_queue : public abstract_ring_queue<T> {
 public:
  explicit concurrent_ring_queue(uindex capacity);
  concurrent_ring_queue(const concurrent_ring_queue&);
  concurrent_ring_queue(concurrent_ring_queue&&) noexcept;
  concurrent_ring_queue& operator=(const concurrent_ring_queue&);
  concurrent_ring_queue& operator=(concurrent_ring_queue&&) noexcept;
  virtual ~concurrent_ring_queue() = default;

  void push(T&& element) override;
  void wait_and_push(T&& element);
  T& front() override;
  T& wait_for_front();
  T wait_and_pop_front();
  void wait_for_data();
  void wait_for_space();
  void pop() noexcept override;
  void clear() noexcept override;

  uindex size() const noexcept override;

 private:
  mutable std::mutex mutex_{};
  std::condition_variable has_data_{};
  uindex head_{0};
  uindex size_{0};
  std::vector<T> elements_{};
};

template <class T>
inline concurrent_ring_queue<T>::concurrent_ring_queue(uindex capacity)
    : abstract_ring_queue<T>{capacity}, elements_{std::vector<T>(capacity)} {}

template <class T>
inline concurrent_ring_queue<T>::concurrent_ring_queue(
    const concurrent_ring_queue<T>& other) {
  auto this_lock{std::unique_lock<std::mutex>(this->mutex_, std::defer_lock)};
  auto other_lock{std::unique_lock<std::mutex>(other.mutex_, std::defer_lock)};
  std::lock(this_lock, other_lock);

  this->capacity_ = other.capacity_;
  this->head_ = other.head_;
  this->size_ = other.size_;
  this->elements_ = other.elements_;
}

template <class T>
inline concurrent_ring_queue<T>::concurrent_ring_queue(
    concurrent_ring_queue<T>&& other) noexcept
    : abstract_ring_queue<T>{std::move(other)},
      head_{other.head_},
      size_{other.size_},
      elements_{std::move(other.elements_)} {
  auto this_lock{std::unique_lock<std::mutex>(this->mutex_, std::defer_lock)};
  auto other_lock{std::unique_lock<std::mutex>(other.mutex_, std::defer_lock)};
  std::lock(this_lock, other_lock);

  other.head_ = 0;
  other.size_ = 0;
  other.elements_clear();
}

template <class T>
inline concurrent_ring_queue<T>& concurrent_ring_queue<T>::operator=(
    const concurrent_ring_queue<T>& other) {
  if (this == &other) {
    return *this;
  }

  auto this_lock{std::unique_lock<std::mutex>(mutex_, std::defer_lock)};
  auto other_lock{std::unique_lock<std::mutex>(other.mutex_, std::defer_lock)};
  std::lock(this_lock, other_lock);

  const auto old_size{this->size_};
  const auto old_capacity{this->capacity_};
  abstract_ring_queue<T>::operator=(other);
  this->head_ = other.head_;
  this->size_ = other.size_;
  this->elements_ = other.elements_;

  if ((old_size == 0 && this->size_ > 0) ||
      (old_size == old_capacity && this->size_ < this->capacity_)) {
    has_data_.notify_one();
  }

  return *this;
}

template <class T>
inline concurrent_ring_queue<T>& concurrent_ring_queue<T>::operator=(
    concurrent_ring_queue<T>&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  auto this_lock{std::unique_lock<std::mutex>(this->mutex_, std::defer_lock)};
  auto other_lock{std::unique_lock<std::mutex>(other.mutex_, std::defer_lock)};
  std::lock(this_lock, other_lock);

  const auto old_size{this->size_};
  const auto old_capacity{this->capacity_};
  abstract_ring_queue<T>::operator=(std::move(other));
  this->head_ = other.head_;
  this->size_ = other.size_;
  this->elements_ = std::move(other.elements_);
  other.head_ = 0;
  other.size_ = 0;
  other.elements_.clear();

  if ((old_size == 0 && this->size_ > 0) ||
      (old_size == old_capacity && this->size_ < this->capacity_)) {
    has_data_.notify_one();
  }

  return *this;
}

template <class T>
inline void concurrent_ring_queue<T>::push(T&& element) {
  std::unique_lock<std::mutex> lock(this->mutex_);

  if (this->size_ == this->capacity_) {
    throw maximum_capacity_reached_error(this->capacity_);
  }

  this->elements_[(this->head_ + this->size_++) % this->capacity_] =
      std::forward<T>(element);
  lock.unlock();
  this->has_data_.notify_one();
}

template <class T>
inline void concurrent_ring_queue<T>::wait_and_push(T&& element) {
  std::unique_lock<std::mutex> lock(mutex_);
  this->has_data_.wait(lock,
                       [this]() { return this->size_ < this->capacity_; });

  this->elements_[(this->head_ + this->size_++) % this->capacity_] =
      std::forward<T>(element);
  lock.unlock();
  this->has_data_.notify_one();
}

template <class T>
inline T& concurrent_ring_queue<T>::front() {
  std::scoped_lock<std::mutex> lock(this->mutex_);

  if (this->size_ == 0) {
    throw empty_error();
  }

  return this->elements_[this->head_];
}

template <class T>
inline T& concurrent_ring_queue<T>::wait_for_front() {
  std::unique_lock<std::mutex> lock(this->mutex_);
  this->has_data_.wait(lock, [this]() { return this->size_ > 0; });

  return this->elements_[this->head_];
}

template <class T>
inline T concurrent_ring_queue<T>::wait_and_pop_front() {
  std::unique_lock<std::mutex> lock(this->mutex_);
  this->has_data_.wait(lock, [this]() { return this->size_ > 0; });

  auto element{std::forward<T>(this->elements_[head_])};

  this->head_ = (this->head_ + 1) % this->capacity_;
  --this->size_;
  lock.unlock();
  this->has_data_.notify_one();

  return element;
}

template <class T>
inline void concurrent_ring_queue<T>::wait_for_data() {
  std::unique_lock<std::mutex> lock(this->mutex_);
  this->has_data_.wait(lock, [this]() { return this->size_ > 0; });
}

template <class T>
inline void concurrent_ring_queue<T>::wait_for_space() {
  std::unique_lock<std::mutex> lock(this->mutex_);
  this->has_data_.wait(lock,
                       [this]() { return this->size_ < this->capacity_; });
}

template <class T>
inline void concurrent_ring_queue<T>::pop() noexcept {
  std::unique_lock<std::mutex> lock(this->mutex_);

  if (this->size_ == 0) {
    return;
  }

  this->head_ = (this->head_ + 1) % this->capacity_;
  --this->size_;
  lock.unlock();
  this->has_data_.notify_one();
}

template <class T>
inline void concurrent_ring_queue<T>::clear() noexcept {
  std::unique_lock<std::mutex> lock(this->mutex_);
  this->head_ = 0;
  this->size_ = 0;
  lock.unlock();
  this->has_data_.notify_one();
}

template <class T>
inline uindex concurrent_ring_queue<T>::size() const noexcept {
  std::scoped_lock<std::mutex> lock(this->mutex_);
  return this->size_;
}
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_STRUCTURE_RING_QUEUE_H_
