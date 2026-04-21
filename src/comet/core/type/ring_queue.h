// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_RING_QUEUE_H_
#define COMET_COMET_CORE_TYPE_RING_QUEUE_H_

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
#include <memory>
#include <optional>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/exception.h"

namespace comet {
template <class T>
class RingQueue {
 public:
  RingQueue() = default;
  explicit RingQueue(memory::Allocator* allocator, usize capacity);
  RingQueue(const RingQueue& other);
  RingQueue(RingQueue&& other) noexcept;
  RingQueue& operator=(const RingQueue& other);
  RingQueue& operator=(RingQueue&& other) noexcept;
  ~RingQueue() = default;

  void Destroy();

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
  Array<T> elements_{};
};

template <class T>
inline RingQueue<T>::RingQueue(memory::Allocator* allocator, usize capacity)
    : capacity_{capacity},
      allocator_{allocator},
      elements_{allocator_, capacity} {
  elements_.Resize(capacity);
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
    : capacity_{other.capacity_},
      head_{other.head_},
      size_{other.size_},
      allocator_{other.allocator_},
      elements_{std::move(other.elements_)} {
  other.capacity_ = 0;
  other.head_ = 0;
  other.size_ = 0;
  other.allocator_ = nullptr;
}

template <class T>
inline RingQueue<T>& RingQueue<T>::operator=(const RingQueue<T>& other) {
  if (this == &other) {
    return *this;
  }

  capacity_ = other.capacity_;
  head_ = other.head_;
  size_ = other.size_;
  allocator_ = other.allocator_;
  elements_ = other.elements_;

  return *this;
}

template <class T>
inline RingQueue<T>& RingQueue<T>::operator=(RingQueue<T>&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  capacity_ = other.capacity_;
  head_ = other.head_;
  size_ = other.size_;
  allocator_ = other.allocator_;
  elements_ = std::move(other.elements_);

  other.capacity_ = 0;
  other.head_ = 0;
  other.size_ = 0;
  other.allocator_ = nullptr;

  return *this;
}

template <class T>
inline void RingQueue<T>::Destroy() {
  elements_.Destroy();
}

template <class T>
inline void RingQueue<T>::Push(T&& element) {
  if (size_ == capacity_) {
    throw MaximumCapacityReachedError(capacity_);
  }

  elements_[(head_ + size_) % capacity_] = std::forward<T>(element);
  ++size_;
}

template <class T>
inline void RingQueue<T>::Push(const T& element) {
  if (size_ == capacity_) {
    throw MaximumCapacityReachedError(capacity_);
  }

  elements_[(head_ + size_) % capacity_] = element;
  ++size_;
}

template <class T>
inline T& RingQueue<T>::Get() {
  if (size_ == 0) {
    throw EmptyError();
  }

  return elements_[head_];
}

template <class T>
inline void RingQueue<T>::TryPop() noexcept {
  if (size_ == 0) {
    return;
  }

  head_ = (head_ + 1) % capacity_;
  --size_;
}

template <class T>
inline void RingQueue<T>::Clear() noexcept {
  head_ = 0;
  size_ = 0;
}

template <class T>
inline bool RingQueue<T>::IsEmpty() const noexcept {
  return size_ == 0;
}

template <class T>
inline usize RingQueue<T>::GetCapacity() const noexcept {
  return capacity_;
}

template <class T>
inline usize RingQueue<T>::GetSize() const noexcept {
  return size_;
}

template <class T>
class LockFreeMPSCRingQueue {
  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> must be always lock-free");

 public:
  LockFreeMPSCRingQueue() = default;
  LockFreeMPSCRingQueue(memory::Allocator* allocator, usize capacity);
  LockFreeMPSCRingQueue(const LockFreeMPSCRingQueue&) = delete;
  LockFreeMPSCRingQueue(LockFreeMPSCRingQueue&& other) noexcept;
  LockFreeMPSCRingQueue& operator=(const LockFreeMPSCRingQueue&) = delete;
  LockFreeMPSCRingQueue& operator=(LockFreeMPSCRingQueue&& other) noexcept;
  ~LockFreeMPSCRingQueue();

  void Destroy();

  void Push(T&& element);
  void Push(const T& element);
  bool TryPop(T& element);
  void Clear();

  usize GetCapacity() const noexcept;

 private:
  struct Slot {
    T element{};
    std::atomic<usize> sequence{0};
  };

  usize capacity_{0};
  std::atomic<usize> head_{0};     // Consumer-owned logical position.
  std::atomic<usize> reserve_{0};  // Producer reservation position.
  memory::Allocator* allocator_{nullptr};
  Slot* elements_{nullptr};

  void DestroyElements() noexcept;
};

template <class T>
inline LockFreeMPSCRingQueue<T>::LockFreeMPSCRingQueue(
    memory::Allocator* allocator, usize capacity)
    : capacity_{capacity}, allocator_{allocator} {
  COMET_ASSERT(capacity_ >= 2, "LockFreeMPSCRingQueue::LockFreeMPSCRingQueue",
               "capacity must be at least 2", "capacity", capacity_);

  elements_ = static_cast<Slot*>(
      allocator_->AllocateAligned(capacity_ * sizeof(Slot), alignof(Slot)));

  for (usize i{0}; i < capacity_; ++i) {
    std::construct_at(&elements_[i]);
    elements_[i].sequence.store(i, std::memory_order_relaxed);
  }

  head_.store(0, std::memory_order_relaxed);
  reserve_.store(0, std::memory_order_relaxed);
}

template <class T>
inline LockFreeMPSCRingQueue<T>::LockFreeMPSCRingQueue(
    LockFreeMPSCRingQueue<T>&& other) noexcept
    : capacity_{other.capacity_},
      head_{other.head_.load(std::memory_order_acquire)},
      reserve_{other.reserve_.load(std::memory_order_acquire)},
      allocator_{other.allocator_},
      elements_{other.elements_} {
  other.capacity_ = 0;
  other.head_.store(0, std::memory_order_relaxed);
  other.reserve_.store(0, std::memory_order_relaxed);
  other.allocator_ = nullptr;
  other.elements_ = nullptr;
}

template <class T>
inline LockFreeMPSCRingQueue<T>& LockFreeMPSCRingQueue<T>::operator=(
    LockFreeMPSCRingQueue<T>&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Destroy();

  capacity_ = other.capacity_;
  head_.store(other.head_.load(std::memory_order_acquire),
              std::memory_order_relaxed);
  reserve_.store(other.reserve_.load(std::memory_order_acquire),
                 std::memory_order_relaxed);
  allocator_ = other.allocator_;
  elements_ = other.elements_;

  other.capacity_ = 0;
  other.head_.store(0, std::memory_order_relaxed);
  other.reserve_.store(0, std::memory_order_relaxed);
  other.allocator_ = nullptr;
  other.elements_ = nullptr;

  return *this;
}

template <class T>
inline LockFreeMPSCRingQueue<T>::~LockFreeMPSCRingQueue() {
  Destroy();
}

template <class T>
inline void LockFreeMPSCRingQueue<T>::DestroyElements() noexcept {
  if (elements_ == nullptr) {
    return;
  }

  for (usize i{0}; i < capacity_; ++i) {
    std::destroy_at(&elements_[i]);
  }
}

template <class T>
inline void LockFreeMPSCRingQueue<T>::Destroy() {
  if (elements_ != nullptr) {
    DestroyElements();
    allocator_->Deallocate(elements_);
    elements_ = nullptr;
  }

  capacity_ = 0;
  head_.store(0, std::memory_order_relaxed);
  reserve_.store(0, std::memory_order_relaxed);
  allocator_ = nullptr;
}

template <class T>
inline void LockFreeMPSCRingQueue<T>::Push(T&& element) {
  usize pos;

  for (;;) {
    pos = reserve_.load(std::memory_order_relaxed);
    auto& slot{elements_[pos % capacity_]};
    const auto seq{slot.sequence.load(std::memory_order_acquire)};
    const auto delta{static_cast<sptrdiff>(seq) - static_cast<sptrdiff>(pos)};

    if (delta == 0) {
      if (reserve_.compare_exchange_weak(pos, pos + 1,
                                         std::memory_order_acq_rel,
                                         std::memory_order_relaxed)) {
        slot.element = std::forward<T>(element);
        slot.sequence.store(pos + 1, std::memory_order_release);
        return;
      }
    } else if (delta < 0) {
      throw MaximumCapacityReachedError(capacity_);
    } else {
      // Another producer moved further ahead. Retry.
    }
  }
}

template <class T>
inline void LockFreeMPSCRingQueue<T>::Push(const T& element) {
  usize pos;

  for (;;) {
    pos = reserve_.load(std::memory_order_relaxed);
    auto& slot{elements_[pos % capacity_]};
    const auto seq{slot.sequence.load(std::memory_order_acquire)};
    const auto delta{static_cast<sptrdiff>(seq) - static_cast<sptrdiff>(pos)};

    if (delta == 0) {
      if (reserve_.compare_exchange_weak(pos, pos + 1,
                                         std::memory_order_acq_rel,
                                         std::memory_order_relaxed)) {
        slot.element = element;
        slot.sequence.store(pos + 1, std::memory_order_release);
        return;
      }
    } else if (delta < 0) {
      throw MaximumCapacityReachedError(capacity_);
    } else {
      // Another producer moved further ahead. Retry.
    }
  }
}

template <class T>
inline bool LockFreeMPSCRingQueue<T>::TryPop(T& element) {
  const auto pos{head_.load(std::memory_order_relaxed)};
  auto& slot{elements_[pos % capacity_]};
  const auto seq{slot.sequence.load(std::memory_order_acquire)};
  const auto delta{static_cast<sptrdiff>(seq) - static_cast<sptrdiff>(pos + 1)};

  if (delta < 0) {
    return false;
  }

  element = std::move(slot.element);
  slot.sequence.store(pos + capacity_, std::memory_order_release);
  head_.store(pos + 1, std::memory_order_relaxed);
  return true;
}

template <class T>
inline void LockFreeMPSCRingQueue<T>::Clear() {
  T element;
  while (TryPop(element)) {
  }
}

template <class T>
inline usize LockFreeMPSCRingQueue<T>::GetCapacity() const noexcept {
  return capacity_;
}

template <class T>
class LockFreeMPMCRingQueue {
  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> must be always lock-free");

 public:
  LockFreeMPMCRingQueue() = default;
  LockFreeMPMCRingQueue(memory::Allocator* allocator, usize capacity);
  LockFreeMPMCRingQueue(const LockFreeMPMCRingQueue&) = delete;
  LockFreeMPMCRingQueue(LockFreeMPMCRingQueue&& other) noexcept;
  LockFreeMPMCRingQueue& operator=(const LockFreeMPMCRingQueue&) = delete;
  LockFreeMPMCRingQueue& operator=(LockFreeMPMCRingQueue&& other) noexcept;
  ~LockFreeMPMCRingQueue();

  void Destroy();

  void Push(const T& element);
  void Push(T&& element);
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

  void DestroyElements() noexcept;

  CachelinePad pad0_{};
  usize mask_{0};
  Node* elements_{nullptr};
  CachelinePad pad1_{};
  std::atomic<usize> head_{0};  // Producer-owned logical position.
  CachelinePad pad2_{};
  std::atomic<usize> tail_{0};  // Consumer-owned logical position.
  CachelinePad pad3_{};
  usize capacity_{0};
  memory::Allocator* allocator_{nullptr};
};

template <class T>
inline LockFreeMPMCRingQueue<T>::LockFreeMPMCRingQueue(
    memory::Allocator* allocator, usize capacity)
    : mask_{capacity - 1}, capacity_{capacity}, allocator_{allocator} {
  COMET_ASSERT(capacity_ >= 2, "LockFreeMPMCRingQueue::LockFreeMPMCRingQueue",
               "capacity must be at least 2", "capacity", capacity_);
  COMET_ASSERT((capacity_ & (capacity_ - 1)) == 0,
               "LockFreeMPMCRingQueue::LockFreeMPMCRingQueue",
               "capacity must be a power of 2", "capacity", capacity_);

  elements_ = static_cast<Node*>(
      allocator_->AllocateAligned(capacity_ * sizeof(Node), alignof(Node)));

  for (usize i{0}; i < capacity_; ++i) {
    std::construct_at(&elements_[i]);
    elements_[i].sequence.store(i, std::memory_order_relaxed);
  }

  head_.store(0, std::memory_order_relaxed);
  tail_.store(0, std::memory_order_relaxed);
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

  Destroy();

  mask_ = other.mask_;
  elements_ = other.elements_;
  head_.store(other.head_.load(std::memory_order_acquire),
              std::memory_order_relaxed);
  tail_.store(other.tail_.load(std::memory_order_acquire),
              std::memory_order_relaxed);
  capacity_ = other.capacity_;
  allocator_ = other.allocator_;

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
  Destroy();
}

template <class T>
inline void LockFreeMPMCRingQueue<T>::DestroyElements() noexcept {
  if (elements_ == nullptr) {
    return;
  }

  for (usize i{0}; i < capacity_; ++i) {
    std::destroy_at(&elements_[i]);
  }
}

template <class T>
inline void LockFreeMPMCRingQueue<T>::Destroy() {
  if (elements_ != nullptr) {
    DestroyElements();
    allocator_->Deallocate(elements_);
    elements_ = nullptr;
  }

  mask_ = 0;
  capacity_ = 0;
  head_.store(0, std::memory_order_relaxed);
  tail_.store(0, std::memory_order_relaxed);
  allocator_ = nullptr;
}

template <class T>
inline void LockFreeMPMCRingQueue<T>::Push(const T& element) {
  Node* node;
  auto pos{head_.load(std::memory_order_relaxed)};

  for (;;) {
    node = &elements_[pos & mask_];
    const auto seq{node->sequence.load(std::memory_order_acquire)};
    const auto delta{static_cast<sptrdiff>(seq) - static_cast<sptrdiff>(pos)};

    if (delta == 0) {
      if (head_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed,
                                      std::memory_order_relaxed)) {
        break;
      }
    } else if (delta < 0) {
      throw MaximumCapacityReachedError(mask_ + 1);
    } else {
      pos = head_.load(std::memory_order_relaxed);
    }
  }

  node->element = element;
  node->sequence.store(pos + 1, std::memory_order_release);
}

template <class T>
inline void LockFreeMPMCRingQueue<T>::Push(T&& element) {
  Node* node;
  auto pos{head_.load(std::memory_order_relaxed)};

  for (;;) {
    node = &elements_[pos & mask_];
    const auto seq{node->sequence.load(std::memory_order_acquire)};
    const auto delta{static_cast<sptrdiff>(seq) - static_cast<sptrdiff>(pos)};

    if (delta == 0) {
      if (head_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed,
                                      std::memory_order_relaxed)) {
        break;
      }
    } else if (delta < 0) {
      throw MaximumCapacityReachedError(mask_ + 1);
    } else {
      pos = head_.load(std::memory_order_relaxed);
    }
  }

  node->element = std::forward<T>(element);
  node->sequence.store(pos + 1, std::memory_order_release);
}

template <class T>
inline std::optional<T> LockFreeMPMCRingQueue<T>::TryPop() {
  Node* node;
  auto pos{tail_.load(std::memory_order_relaxed)};

  for (;;) {
    node = &elements_[pos & mask_];
    const auto seq{node->sequence.load(std::memory_order_acquire)};
    const auto delta{static_cast<sptrdiff>(seq) -
                     static_cast<sptrdiff>(pos + 1)};

    if (delta == 0) {
      if (tail_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed,
                                      std::memory_order_relaxed)) {
        break;
      }
    } else if (delta < 0) {
      return std::nullopt;
    } else {
      pos = tail_.load(std::memory_order_relaxed);
    }
  }

  const auto element{std::move(node->element)};
  node->sequence.store(pos + mask_ + 1, std::memory_order_release);
  return element;
}

template <class T>
inline void LockFreeMPMCRingQueue<T>::Clear() {
  while (TryPop().has_value()) {
  }
}

template <class T>
inline usize LockFreeMPMCRingQueue<T>::GetCapacity() const noexcept {
  return capacity_;
}
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_RING_QUEUE_H_
