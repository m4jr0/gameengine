// Copyright 2021 m4jr0. All Rights Reserved.
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
  virtual void push(T&& element) = 0;
  virtual T& front() = 0;
  virtual void pop() noexcept = 0;
  virtual void clear() noexcept = 0;

  // By default, there is not empty() member to prevent from using it in the
  // thread-safe implementation and therefore having TOCTTOU bugs.
  virtual std::size_t size() const noexcept = 0;
  virtual std::size_t capacity() const noexcept;

 protected:
  std::size_t capacity_ = 0;
};

template <class T>
inline std::size_t abstract_ring_queue<T>::capacity() const noexcept {
  return capacity_;
}

// Non thread-safe ring queue with a fixed capacity given at its instanciation.
// The public member naming rules follow the STL containers to ease its use.
template <class T>
class ring_queue : public abstract_ring_queue<T> {
 public:
  ring_queue(std::size_t);

  virtual void push(T&& element) override;
  virtual T& front() override;
  virtual void pop() noexcept override;
  virtual void clear() noexcept override;
  virtual bool empty() const noexcept;

  std::size_t size() const noexcept override;

 private:
  std::size_t head_ = 0;
  std::size_t size_ = 0;
  std::vector<T> elements_;
};

template <class T>
inline ring_queue<T>::ring_queue(std::size_t capacity)
    : elements_(std::vector<T>(capacity)) {
  this->capacity_ = capacity;
}

template <class T>
inline std::size_t ring_queue<T>::size() const noexcept {
  return size_;
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

// Thread-safe ring queue with a fixed capacity given at its instanciation.
// The public member naming rules follow the STL containers to ease its use.
template <class T>
class concurrent_ring_queue : public abstract_ring_queue<T> {
 public:
  concurrent_ring_queue(std::size_t);

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
    : elements_(std::vector<T>(capacity)) {
  this->capacity_ = capacity;
}

template <class T>
inline std::size_t concurrent_ring_queue<T>::size() const noexcept {
  std::scoped_lock<std::mutex> lock(mutex_);
  return size_;
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
}  // namespace structure
}  // namespace utils
}  // namespace comet

#endif  // COMET_COMET_UTILS_STRUCTURE_RING_QUEUE_H_
