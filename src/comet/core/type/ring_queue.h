// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_RING_QUEUE_H_
#define COMET_COMET_CORE_TYPE_RING_QUEUE_H_

#include <atomic>
#include <optional>
#include <vector>

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/memory_utils.h"
#include "exception.h"

namespace comet {
template <class T>
class RingQueue {
 public:
  RingQueue() = default;
  explicit RingQueue(memory::Allocator* allocator, usize capacity);
  RingQueue(const RingQueue& other);
  RingQueue(RingQueue&& other) noexcept;
  RingQueue& operator=(const RingQueue&);
  RingQueue& operator=(RingQueue&& other) noexcept;
  ~RingQueue() = default;

  void Push(T&& element);
  void Push(const T& element);
  T& Get();
  void TryPop() noexcept;
  void Clear() noexcept;
  bool IsEmpty() const noexcept;

  usize GetCapacity() const noexcept;
  usize GetSize() const noexcept;

 private:
  usize capacity_{0};
  usize head_{0};
  usize size_{0};
  memory::Allocator* allocator_{nullptr};
  std::vector<T> elements_{};
};

template <class T>
inline RingQueue<T>::RingQueue(memory::Allocator* allocator,
                               usize capacity)
    : capacity_{capacity},
      allocator_{allocator},
      elements_{std::vector<T>(capacity)} {
  // TODO(m4jr0): Replace vector with custom class and use allocator provided.
}

template <class T>
inline RingQueue<T>::RingQueue(const RingQueue<T>& other)
    : capacity_{other.capacity_},
      head_{other.head_},
      size_{other.size_},
      allocator_{other.allocator_},
      elements_{other.elements_} {}

template <class T>
inline RingQueue<T>::RingQueue(RingQueue<T>&& other) noexcept
    : capacity_{(other.capacity_)},
      head_{other.head_},
      size_{other.size_},
      allocator_{other.allocator_},
      elements_{std::move(other.elements_)} {
  other.capacity_ = 0;
  other.head_ = 0;
  other.size_ = 0;
  other.allocator_ = nullptr;
  other.elements_.clear();
}

template <class T>
inline RingQueue<T>& RingQueue<T>::operator=(const RingQueue<T>& other) {
  if (this == &other) {
    return *this;
  }

  this->capacity_ = other.capacity_;
  this->head_ = other.head_;
  this->size_ = other.size_;
  this->allocator_ = other.allocator_;
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
  this->allocator_ = other.allocator_;
  this->elements_ = std::move(other.elements_);
  other.capacity_ = 0;
  other.head_ = 0;
  other.size_ = 0;
  other.allocator_ = nullptr;
  other.elements_.clear();

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
inline void RingQueue<T>::TryPop() noexcept {
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
  LockFreeMPSCRingQueue() = default;
  LockFreeMPSCRingQueue(memory::Allocator* allocator, usize capacity);
  LockFreeMPSCRingQueue(const LockFreeMPSCRingQueue& other);
  LockFreeMPSCRingQueue(LockFreeMPSCRingQueue&& other) noexcept;
  LockFreeMPSCRingQueue& operator=(const LockFreeMPSCRingQueue& other);
  LockFreeMPSCRingQueue& operator=(LockFreeMPSCRingQueue&& other) noexcept;
  ~LockFreeMPSCRingQueue();

  void Push(T&& element);
  void Push(const T& element);
  bool TryPop(T& element);
  void Clear();

 private:
  usize capacity_{0};
  std::atomic<usize> head_{0};
  std::atomic<usize> tail_{0};
  memory::Allocator* allocator_{nullptr};
  std::vector<T> elements_{};

  usize Increment(usize index) const;
};

template <class T>
inline LockFreeMPSCRingQueue<T>::LockFreeMPSCRingQueue(
    memory::Allocator* allocator, usize capacity)
    : capacity_{capacity},
      allocator_{allocator},
      elements_{std::vector<T>(capacity)} {
  // TODO(m4jr0): Replace vector with custom class and use allocator provided.
}

template <class T>
inline LockFreeMPSCRingQueue<T>::LockFreeMPSCRingQueue(
    const LockFreeMPSCRingQueue<T>& other)
    : capacity_{other.capacity_},
      head_{other.head_.load(std::memory_order_acquire)},
      tail_{other.tail_.load(std::memory_order_acquire)},
      allocator_{other.allocator_},
      elements_{other.elements_} {}

template <class T>
inline LockFreeMPSCRingQueue<T>::LockFreeMPSCRingQueue(
    LockFreeMPSCRingQueue<T>&& other) noexcept
    : capacity_{other.capacity_},
      head_{other.head_.load(std::memory_order_acquire)},
      tail_{other.tail_.load(std::memory_order_acquire)},
      allocator_{other.allocator_},
      elements_{std::move(other.elements_)} {
  other.capacity_ = 0;
  other.head_.store(0, std::memory_order_relaxed);
  other.tail_.store(0, std::memory_order_relaxed);
  other.allocator_ = nullptr;
  other.elements_.Clear();
}

template <class T>
inline LockFreeMPSCRingQueue<T>& LockFreeMPSCRingQueue<T>::operator=(
    const LockFreeMPSCRingQueue<T>& other) {
  if (this == &other) {
    return *this;
  }

  this->capacity_ = other.capacity_;
  this->head_.store(other.head_.load(std::memory_order_acquire),
                    std::memory_order_release);
  this->tail_.store(other.tail_.load(std::memory_order_acquire),
                    std::memory_order_release);
  this->allocator_ = other.allocator_;
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
  this->head_.store(other.head_.load(std::memory_order_acquire),
                    std::memory_order_release);
  this->tail_.store(other.tail_.load(std::memory_order_acquire),
                    std::memory_order_release);
  this->allocator_ = other.allocator_;
  this->elements_ = std::move(other.elements_);
  other.capacity_ = 0;
  other.head_.store(0, std::memory_order_relaxed);
  other.tail_.store(0, std::memory_order_relaxed);
  other.allocator_ = nullptr;
  other.elements_.clear();

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
inline bool LockFreeMPSCRingQueue<T>::TryPop(T& element) {
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

  while (TryPop(element))
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
  LockFreeMPMCRingQueue() = default;
  LockFreeMPMCRingQueue(memory::Allocator* allocator, usize capacity);
  LockFreeMPMCRingQueue(const LockFreeMPMCRingQueue&) = delete;
  LockFreeMPMCRingQueue(LockFreeMPMCRingQueue&& other) noexcept;
  LockFreeMPMCRingQueue& operator=(const LockFreeMPMCRingQueue&) = delete;
  LockFreeMPMCRingQueue& operator=(LockFreeMPMCRingQueue&& other) noexcept;
  ~LockFreeMPMCRingQueue();

  void Push(const T& element);
  std::optional<T> TryPop();
  void Clear();

  usize GetCapacity() const noexcept;

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
  usize capacity_{0};
  memory::Allocator* allocator_{nullptr};
};

template <class T>
inline LockFreeMPMCRingQueue<T>::LockFreeMPMCRingQueue(
    memory::Allocator* allocator, usize capacity)
    : mask_{capacity - 1},
      elements_{static_cast<Node*>(
          allocator->AllocateAligned(capacity * sizeof(Node), alignof(Node)))},
      capacity_{capacity},
      allocator_{allocator} {
  COMET_ASSERT(this->capacity_ >= 2,
               "Capacity needs to be at least 2: ", this->capacity_, "!");
  COMET_ASSERT((this->capacity_ & (this->capacity_ - 1)) == 0,
               "Capacity must be a power of 2: ", this->capacity_, "!");

  for (usize i{0}; i < this->capacity_; ++i) {
    this->elements_[i].sequence.store(i, std::memory_order_relaxed);
  }

  this->head_.store(0, std::memory_order_relaxed);
  this->tail_.store(0, std::memory_order_relaxed);
}

template <class T>
inline LockFreeMPMCRingQueue<T>::LockFreeMPMCRingQueue(
    LockFreeMPMCRingQueue<T>&& other) noexcept
    : mask_{other.mask_},
      elements_{other.elements_},
      head_{other.head_.load(std::memory_order_acquire)},
      tail_{other.tail_.load(std::memory_order_acquire)},
      capacity_{other.capacity_},
      allocator_{other.allocator_} {
  other.mask_ = 0;
  other.elements_ = nullptr;
  other.head_.store(0, std::memory_order_relaxed);
  other.tail_.store(0, std::memory_order_relaxed);
  other.capacity_ = 0;
  other.allocator_ = nullptr;
}

template <class T>
inline LockFreeMPMCRingQueue<T>& LockFreeMPMCRingQueue<T>::operator=(
    LockFreeMPMCRingQueue<T>&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  if (this->elements_ != nullptr) {
    memory::Deallocate(this->elements_);
  }

  this->mask_ = other.mask_;
  this->elements_ = other.elements_;
  this->head_ = other.head_.load(std::memory_order_acquire);
  this->tail_ = other.tail_.load(std::memory_order_acquire);
  this->capacity_ = other.capacity_;
  this->allocator_ = other.allocator_;

  other.mask_ = 0;
  other.elements_ = nullptr;
  other.head_.store(0, std::memory_order_relaxed);
  other.tail_.store(0, std::memory_order_relaxed);
  other.capacity_ = 0;
  other.allocator_ = nullptr;
  return *this;
}

template <class T>
inline LockFreeMPMCRingQueue<T>::~LockFreeMPMCRingQueue() {
  if (this->elements_ != nullptr) {
    allocator_->Deallocate(this->elements_);
  }
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
inline std::optional<T> LockFreeMPMCRingQueue<T>::TryPop() {
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
  while (TryPop().has_value())
    ;
}
template <class T>
inline usize LockFreeMPMCRingQueue<T>::GetCapacity() const noexcept {
  return capacity_;
}
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_RING_QUEUE_H_
