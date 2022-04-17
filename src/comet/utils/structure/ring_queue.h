// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_UTILS_STRUCTURE_RING_QUEUE_H_
#define COMET_COMET_UTILS_STRUCTURE_RING_QUEUE_H_

#include "comet_precompile.h"
#include "exception.h"

namespace comet {
namespace utils {
namespace structure {
// Abstract ring queue with a fixed capacity given at its instanciation.
// The public member naming rules follow the STL containers to ease its use.
template <class T>
class abstract_ring_queue {
 public:
  explicit abstract_ring_queue(std::size_t);
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

  virtual std::size_t capacity() const noexcept;
  virtual std::size_t size() const noexcept = 0;

 protected:
  std::size_t capacity_ = 0;
};

template <class T>
inline abstract_ring_queue<T>::abstract_ring_queue(std::size_t capacity)
    : capacity_(capacity) {}

template <class T>
inline abstract_ring_queue<T>::abstract_ring_queue(
    const abstract_ring_queue<T>& other)
    : capacity_(other.capacity_) {}

template <class T>
inline abstract_ring_queue<T>::abstract_ring_queue(
    abstract_ring_queue<T>&& other) noexcept
    : capacity_(std::move(other.capacity_)) {}

template <class T>
inline abstract_ring_queue<T>& abstract_ring_queue<T>::operator=(
    const abstract_ring_queue<T>& other) {
  if (this == &other) {
    return *this;
  }

  capacity_ = other.capacity;
  return *this;
}

template <class T>
inline abstract_ring_queue<T>& abstract_ring_queue<T>::operator=(
    abstract_ring_queue<T>&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  capacity_ = std::move(capacity_);
  return *this;
}

template <class T>
inline std::size_t abstract_ring_queue<T>::capacity() const noexcept {
  return capacity_;
}

// Non thread-safe ring queue with a fixed capacity given at its instanciation.
// The public member naming rules follow the STL containers to ease its use.
template <class T>
class ring_queue : public abstract_ring_queue<T> {
 public:
  explicit ring_queue(std::size_t);
  ring_queue(const ring_queue&);
  ring_queue(ring_queue&&) noexcept;
  ring_queue& operator=(const ring_queue&);
  ring_queue& operator=(ring_queue&&) noexcept;
  virtual ~ring_queue() = default;

  virtual void push(T&& element) override;
  virtual T& front() override;
  virtual void pop() noexcept override;
  virtual void clear() noexcept override;
  virtual bool empty() const noexcept;

  virtual std::size_t size() const noexcept override;

 private:
  std::size_t head_ = 0;
  std::size_t size_ = 0;
  std::vector<T> elements_;
};

template <class T>
inline ring_queue<T>::ring_queue(std::size_t capacity)
    : abstract_ring_queue<T>(capacity), elements_(std::vector<T>(capacity)) {}

template <class T>
inline ring_queue<T>::ring_queue(const ring_queue<T>& other)
    : abstract_ring_queue<T>(other),
      head_(other.head_),
      size_(other.size_),
      elements_(other.elements_) {}

template <class T>
inline ring_queue<T>::ring_queue(ring_queue<T>&& other) noexcept
    : abstract_ring_queue<T>(std::move(other)),
      head_(std::move(other.head_)),
      size_(std::move(other.size_)),
      elements_(std::move(other.elements_)) {}

template <class T>
inline ring_queue<T>& ring_queue<T>::operator=(const ring_queue<T>& other) {
  if (this == &other) {
    return *this;
  }

  abstract_ring_queue<T>::operator=(other);
  head_ = other.head_;
  size_ = other.size_;
  elements_ = other.elements_;

  return *this;
}

template <class T>
inline ring_queue<T>& ring_queue<T>::operator=(ring_queue<T>&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  abstract_ring_queue<T>::operator=(std::move(other));
  head_ = std::move(other.head_);
  size_ = std::move(other.size_);
  elements_ = std::move(other.elements_);

  return *this;
}

template <class T>
inline void ring_queue<T>::push(T&& element) {
  if (size_ == this->capacity_) {
    throw maximum_capacity_reached_error(this->capacity_);
  }

  elements_[(head_ + size_++) % this->capacity_] = std::forward<T>(element);
}

template <class T>
inline T& ring_queue<T>::front() {
  if (size_ == 0) {
    throw empty_error();
  }

  return elements_[head_];
}

template <class T>
inline void ring_queue<T>::pop() noexcept {
  if (size_ == 0) {
    return;
  }

  head_ = (head_ + 1) % this->capacity_;
  --size_;
}

template <class T>
inline void ring_queue<T>::clear() noexcept {
  head_ = 0;
  size_ = 0;
}

template <class T>
inline bool ring_queue<T>::empty() const noexcept {
  return size_ == 0;
}

template <class T>
inline std::size_t ring_queue<T>::size() const noexcept {
  return size_;
}

// Thread-safe ring queue with a fixed capacity given at its instanciation.
// The public member naming rules follow the STL containers to ease its use.
template <class T>
class concurrent_ring_queue : public abstract_ring_queue<T> {
 public:
  explicit concurrent_ring_queue(std::size_t);
  concurrent_ring_queue(const concurrent_ring_queue&);
  concurrent_ring_queue(concurrent_ring_queue&&) noexcept;
  concurrent_ring_queue& operator=(const concurrent_ring_queue&);
  concurrent_ring_queue& operator=(concurrent_ring_queue&&) noexcept;
  virtual ~concurrent_ring_queue() = default;

  virtual void push(T&& element) override;
  virtual void wait_and_push(T&& element);
  virtual T& front() override;
  virtual T& wait_for_front();
  virtual T wait_and_pop_front();
  virtual void wait_for_data();
  virtual void wait_for_space();
  virtual void pop() noexcept override;
  virtual void clear() noexcept override;

  virtual std::size_t size() const noexcept override;

 private:
  mutable std::mutex mutex_;
  std::condition_variable has_data_;
  std::size_t head_ = 0;
  std::size_t size_ = 0;
  std::vector<T> elements_;
};

template <class T>
inline concurrent_ring_queue<T>::concurrent_ring_queue(std::size_t capacity)
    : abstract_ring_queue<T>(capacity), elements_(std::vector<T>(capacity)) {}

template <class T>
inline concurrent_ring_queue<T>::concurrent_ring_queue(
    const concurrent_ring_queue<T>& other) {
  auto this_lock = std::unique_lock<std::mutex>(mutex_, std::defer_lock);
  auto other_lock = std::unique_lock<std::mutex>(other.mutex_, std::defer_lock);
  std::lock(this_lock, other_lock);

  this->capacity_ = this->capacity_;
  head_ = other.head_;
  size_ = other.size_;
  elements_ = other.elements_;
}

template <class T>
inline concurrent_ring_queue<T>::concurrent_ring_queue(
    concurrent_ring_queue<T>&& other) noexcept {
  auto this_lock = std::unique_lock<std::mutex>(mutex_, std::defer_lock);
  auto other_lock = std::unique_lock<std::mutex>(other.mutex_, std::defer_lock);
  std::lock(this_lock, other_lock);

  this->capacity_ = std::move(this->capacity_);
  head_ = std::move(other.head_);
  size_ = std::move(other.size_);
  elements_ = std::move(other.elements_);
}

template <class T>
inline concurrent_ring_queue<T>& concurrent_ring_queue<T>::operator=(
    const concurrent_ring_queue<T>& other) {
  if (this == &other) {
    return *this;
  }

  auto this_lock = std::unique_lock<std::mutex>(mutex_, std::defer_lock);
  auto other_lock = std::unique_lock<std::mutex>(other.mutex_, std::defer_lock);
  std::lock(this_lock, other_lock);

  const auto old_size = size_;
  const auto old_capacity = capacity_;
  abstract_ring_queue<T>::operator=(other);
  head_ = other.head_;
  size_ = other.size_;
  elements_ = other.elements_;

  if ((old_size == 0 && size_ > 0) ||
      (old_size == old_capacity && size_ < capacity_)) {
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

  auto this_lock = std::unique_lock<std::mutex>(mutex_, std::defer_lock);
  auto other_lock = std::unique_lock<std::mutex>(other.mutex_, std::defer_lock);
  std::lock(this_lock, other_lock);

  abstract_ring_queue<T>::operator=(std::move(other));
  head_ = std::move(other.head_);
  size_ = std::move(other.size_);
  elements_ = std::move(other.elements_);

  if ((old_size == 0 && size_ > 0) ||
      (old_size == old_capacity && size_ < capacity_)) {
    has_data_.notify_one();
  }

  return *this;
}

template <class T>
inline void concurrent_ring_queue<T>::push(T&& element) {
  std::unique_lock<std::mutex> lock(mutex_);

  if (size_ == this->capacity_) {
    throw maximum_capacity_reached_error(this->capacity_);
  }

  elements_[(head_ + size_++) % this->capacity_] = std::forward<T>(element);
  lock.unlock();
  has_data_.notify_one();
}

template <class T>
inline void concurrent_ring_queue<T>::wait_and_push(T&& element) {
  std::unique_lock<std::mutex> lock(mutex_);
  has_data_.wait(lock, [this]() { return size_ < this->capacity_; });

  elements_[(head_ + size_++) % this->capacity_] = std::forward<T>(element);
  lock.unlock();
  has_data_.notify_one();
}

template <class T>
inline T& concurrent_ring_queue<T>::front() {
  std::scoped_lock<std::mutex> lock(mutex_);

  if (size_ == 0) {
    throw empty_error();
  }

  return elements_[head_];
}

template <class T>
inline T& concurrent_ring_queue<T>::wait_for_front() {
  std::unique_lock<std::mutex> lock(mutex_);
  has_data_.wait(lock, [this]() { return size_ > 0; });

  return elements_[head_];
}

template <class T>
inline T concurrent_ring_queue<T>::wait_and_pop_front() {
  std::unique_lock<std::mutex> lock(mutex_);
  has_data_.wait(lock, [this]() { return size_ > 0; });

  auto element = std::forward<T>(elements_[head_]);

  head_ = (head_ + 1) % this->capacity_;
  --size_;
  lock.unlock();
  has_data_.notify_one();

  return element;
}

template <class T>
inline void concurrent_ring_queue<T>::wait_for_data() {
  std::unique_lock<std::mutex> lock(mutex_);
  has_data_.wait(lock, [this]() { return size_ > 0; });
}

template <class T>
inline void concurrent_ring_queue<T>::wait_for_space() {
  std::unique_lock<std::mutex> lock(mutex_);
  has_data_.wait(lock, [this]() { return size_ < this->capacity_; });
}

template <class T>
inline void concurrent_ring_queue<T>::pop() noexcept {
  std::unique_lock<std::mutex> lock(mutex_);

  if (size_ == 0) {
    return;
  }

  head_ = (head_ + 1) % this->capacity_;
  --size_;
  lock.unlock();
  has_data_.notify_one();
}

template <class T>
inline void concurrent_ring_queue<T>::clear() noexcept {
  std::unique_lock<std::mutex> lock(mutex_);
  head_ = 0;
  size_ = 0;
  lock.unlock();
  has_data_.notify_one();
}

template <class T>
inline std::size_t concurrent_ring_queue<T>::size() const noexcept {
  std::scoped_lock<std::mutex> lock(mutex_);
  return size_;
}
}  // namespace structure
}  // namespace utils
}  // namespace comet

#endif  // COMET_COMET_UTILS_STRUCTURE_RING_QUEUE_H_
