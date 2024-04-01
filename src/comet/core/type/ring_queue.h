// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_RING_QUEUE_H_
#define COMET_COMET_CORE_TYPE_RING_QUEUE_H_

#include <atomic>
#include <optional>
#include <vector>

#include "comet/core/essentials.h"
#include "exception.h"

namespace comet {
template <class T>
class RingQueue {
 public:
  explicit RingQueue(usize);
  RingQueue(const RingQueue&);
  RingQueue(RingQueue&&) noexcept;
  RingQueue& operator=(const RingQueue&);
  RingQueue& operator=(RingQueue&&) noexcept;
  ~RingQueue() = default;

  void Push(T&& element);
  void Push(const T& element);
  T& Get();
  void Pop() noexcept;
  void Clear() noexcept;
  bool IsEmpty() const noexcept;

  usize GetCapacity() const noexcept;
  usize GetSize() const noexcept;

 private:
  usize capacity_{0};
  usize head_{0};
  usize size_{0};
  std::vector<T> elements_{};
};

template <class T>
inline RingQueue<T>::RingQueue(usize capacity)
    : capacity_{capacity}, elements_{std::vector<T>(capacity)} {}

template <class T>
inline RingQueue<T>::RingQueue(const RingQueue<T>& other)
    : capacity_{other.capacity_},
      head_{other.head_},
      size_{other.size_},
      elements_{other.elements_} {}

template <class T>
inline RingQueue<T>::RingQueue(RingQueue<T>&& other) noexcept
    : capacity_{(other.capacity_)},
      head_{other.head_},
      size_{other.size_},
      elements_{std::move(other.elements_)} {
  other.capacity_ = 0;
  other.head_ = 0;
  other.size_ = 0;
  other.elements_.Clear();
}

template <class T>
inline RingQueue<T>& RingQueue<T>::operator=(const RingQueue<T>& other) {
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
inline RingQueue<T>& RingQueue<T>::operator=(RingQueue<T>&& other) noexcept {
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
  other.elements_.Clear();

  return *this;
}

template <class T>
inline void RingQueue<T>::Push(T&& element) {
  if (this->size_ == this->capacity_) {
    throw MaximumCapacityReachedError(this->capacity_);
  }

  this->elements_[(this->head_ + this->size_++) % this->capacity_] =
      std::forward<T>(element);
}

template <class T>
inline void RingQueue<T>::Push(const T& element) {
  if (this->size_ == this->capacity_) {
    throw MaximumCapacityReachedError(this->capacity_);
  }

  this->elements_[(this->head_ + this->size_++) % this->capacity_] = element;
}

template <class T>
inline T& RingQueue<T>::Get() {
  if (this->size_ == 0) {
    throw EmptyError();
  }

  return this->elements_[this->head_];
}

template <class T>
inline void RingQueue<T>::Pop() noexcept {
  if (this->size_ == 0) {
    return;
  }

  this->head_ = (this->head_ + 1) % this->capacity_;
  --this->size_;
}

template <class T>
inline void RingQueue<T>::Clear() noexcept {
  this->head_ = 0;
  this->size_ = 0;
}

template <class T>
inline bool RingQueue<T>::IsEmpty() const noexcept {
  return this->size_ == 0;
}

template <class T>
inline usize RingQueue<T>::GetCapacity() const noexcept {
  return this->capacity_;
}

template <class T>
inline usize RingQueue<T>::GetSize() const noexcept {
  return this->size_;
}

template <class T>
class LockFreeMPSCRingQueue {
  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> needs to be always lock-free. Unsupported "
                "architecture");

 public:
  explicit LockFreeMPSCRingQueue(usize);
  LockFreeMPSCRingQueue(const LockFreeMPSCRingQueue&);
  LockFreeMPSCRingQueue(LockFreeMPSCRingQueue&&) noexcept;
  LockFreeMPSCRingQueue& operator=(const LockFreeMPSCRingQueue&);
  LockFreeMPSCRingQueue& operator=(LockFreeMPSCRingQueue&&) noexcept;
  ~LockFreeMPSCRingQueue();

  void Push(T&& element);
  void Push(const T& element);
  bool Pop(T& element);
  void Clear();

 private:
  usize capacity_{0};
  std::atomic<usize> head_{0};
  std::atomic<usize> tail_{0};
  usize size_{0};
  std::vector<T> elements_{};

  usize Increment(usize index) const;
};

template <class T>
inline LockFreeMPSCRingQueue<T>::LockFreeMPSCRingQueue(usize capacity)
    : capacity_{capacity}, elements_{std::vector<T>(capacity)} {}

template <class T>
inline LockFreeMPSCRingQueue<T>::LockFreeMPSCRingQueue(
    const LockFreeMPSCRingQueue<T>& other)
    : capacity_{other.capacity_},
      head_{other.head_},
      size_{other.size_},
      elements_{other.elements_} {}

template <class T>
inline LockFreeMPSCRingQueue<T>::LockFreeMPSCRingQueue(
    LockFreeMPSCRingQueue<T>&& other) noexcept
    : capacity_{other.capacity_},
      head_{other.head_},
      size_{other.size_},
      elements_{std::move(other.elements_)} {
  other.capacity_ = 0;
  other.head_ = 0;
  other.size_ = 0;
  other.elements_.Clear();
}

template <class T>
inline LockFreeMPSCRingQueue<T>& LockFreeMPSCRingQueue<T>::operator=(
    const LockFreeMPSCRingQueue<T>& other) {
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
inline LockFreeMPSCRingQueue<T>& LockFreeMPSCRingQueue<T>::operator=(
    LockFreeMPSCRingQueue<T>&& other) noexcept {
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
  other.elements_.Clear();

  return *this;
}

template <class T>
inline LockFreeMPSCRingQueue<T>::~LockFreeMPSCRingQueue() {
  Clear();
}

template <class T>
inline void LockFreeMPSCRingQueue<T>::Push(T&& element) {
  usize current_tail;
  usize next_tail;

  do {
    current_tail = this->tail_.load(std::memory_order_relaxed);
    next_tail = Increment(current_tail);

    if (next_tail == this->head_.load(std::memory_order_acquire)) {
      throw MaximumCapacityReachedError(this->capacity_);
    }
  } while (!this->tail_.compare_exchange_weak(current_tail, next_tail,
                                              std::memory_order_release));

  this->elements_[current_tail] = std::forward<T>(element);
}

template <class T>
inline void LockFreeMPSCRingQueue<T>::Push(const T& element) {
  usize current_tail;
  usize next_tail;

  do {
    current_tail = this->tail_.load(std::memory_order_relaxed);
    next_tail = Increment(current_tail);

    if (next_tail == this->head_.load(std::memory_order_acquire)) {
      throw MaximumCapacityReachedError(this->capacity_);
    }
  } while (!this->tail_.compare_exchange_weak(current_tail, next_tail,
                                              std::memory_order_release));

  this->elements_[current_tail] = element;
}

template <class T>
inline bool LockFreeMPSCRingQueue<T>::Pop(T& element) {
  auto current_head = this->head_.load(std::memory_order_relaxed);

  if (current_head == this->tail_.load(std::memory_order_acquire)) {
    return false;
  }

  element = std::move(this->elements_[current_head]);
  this->head_.store(Increment(current_head), std::memory_order_release);
  return true;
}

template <class T>
inline void LockFreeMPSCRingQueue<T>::Clear() {
  T element;

  while (Pop(element))
    ;
}

template <class T>
inline usize LockFreeMPSCRingQueue<T>::Increment(usize index) const {
  return (index + 1) % elements_.size();
}

// https://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
template <class T>
class LockFreeMPMCRingQueue {
  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> needs to be always lock-free. Unsupported "
                "architecture");

 public:
  explicit LockFreeMPMCRingQueue(usize capacity);
  ~LockFreeMPMCRingQueue();

  void Push(const T& element);
  std::optional<T> Pop();
  void Clear();

 private:
  struct Node {
    T element{};
    std::atomic<usize> sequence{0};
  };

  constexpr static auto kCachelineSize_{64};
  using CachelinePad = u8[kCachelineSize_];

  CachelinePad pad0_{};
  usize mask_{0};
  Node* elements_{nullptr};
  CachelinePad pad1_{};
  std::atomic<usize> head_{0};
  CachelinePad pad2_{};
  std::atomic<usize> tail_{0};
  CachelinePad pad3_{};
};

template <class T>
inline LockFreeMPMCRingQueue<T>::LockFreeMPMCRingQueue(usize capacity)
    : mask_{capacity - 1}, elements_(new Node[capacity]) {
  COMET_ASSERT(capacity >= 2, "Capacity needs to be at least 2: ", capacity,
               "!");
  COMET_ASSERT((capacity & (capacity - 1)) == 0,
               "Capacity must be a power of 2: ", capacity, "!");

  for (usize i{0}; i < capacity; ++i) {
    this->elements_[i].sequence.store(i, std::memory_order_relaxed);
  }

  this->head_.store(0, std::memory_order_relaxed);
  this->tail_.store(0, std::memory_order_relaxed);
}

template <class T>
inline LockFreeMPMCRingQueue<T>::~LockFreeMPMCRingQueue() {
  delete[] this->elements_;
}

template <class T>
inline void LockFreeMPMCRingQueue<T>::Push(const T& element) {
  Node* node;
  auto pos{this->head_.load(std::memory_order_relaxed)};

  for (;;) {
    node = &this->elements_[pos & this->mask_];
    auto seq{node->sequence.load(std::memory_order_acquire)};
    auto delta{static_cast<sptrdiff>(seq) - static_cast<sptrdiff>(pos)};

    if (delta == 0) {
      if (this->head_.compare_exchange_weak(pos, pos + 1,
                                            std::memory_order_relaxed)) {
        break;
      }
    } else if (delta < 0) {
      throw MaximumCapacityReachedError(this->mask_ + 1);
    } else {
      pos = this->head_.load(std::memory_order_relaxed);
    }
  }

  node->element = element;
  node->sequence.store(pos + 1, std::memory_order_release);
}

template <class T>
inline std::optional<T> LockFreeMPMCRingQueue<T>::Pop() {
  Node* node;
  auto pos{this->tail_.load(std::memory_order_relaxed)};

  for (;;) {
    node = &this->elements_[pos & this->mask_];
    auto seq{node->sequence.load(std::memory_order_acquire)};
    auto delta{static_cast<sptrdiff>(seq) - static_cast<sptrdiff>(pos + 1)};

    if (delta == 0) {
      if (this->tail_.compare_exchange_weak(pos, pos + 1,
                                            std::memory_order_relaxed)) {
        break;
      }
    } else if (delta < 0) {
      return std::nullopt;
    } else {
      pos = this->tail_.load(std::memory_order_relaxed);
    }
  }

  auto element{node->element};
  node->sequence.store(pos + this->mask_ + 1, std::memory_order_release);
  return element;
}

template <class T>
inline void LockFreeMPMCRingQueue<T>::Clear() {
  while (Pop().has_value())
    ;
}
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_RING_QUEUE_H_
