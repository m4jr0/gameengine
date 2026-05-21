// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_CONTAINER_RING_QUEUE_H_
#define COMET_CORE_CONTAINER_RING_QUEUE_H_

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
#include <memory>
#include <optional>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/thread/thread_common.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"

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

  bool TryPush(T&& element);
  bool TryPush(const T& element);
  void Push(T&& element);
  void Push(const T& element);

  T* TryGet() noexcept;
  const T* TryGet() const noexcept;
  T& Get();
  const T& Get() const;

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
    : capacity_{capacity}, allocator_{allocator}, elements_{allocator_} {
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
  capacity_ = 0;
  head_ = 0;
  size_ = 0;
  allocator_ = nullptr;
}

template <class T>
inline bool RingQueue<T>::TryPush(T&& element) {
  if (size_ == capacity_) {
    return false;
  }

  COMET_ASSERT(capacity_ > 0, "RingQueue::TryPush",
               "capacity is zero while queue is not full");

  elements_[(head_ + size_) % capacity_] = std::move(element);
  ++size_;
  return true;
}

template <class T>
inline bool RingQueue<T>::TryPush(const T& element) {
  if (size_ == capacity_) {
    return false;
  }

  COMET_ASSERT(capacity_ > 0, "RingQueue::TryPush",
               "capacity is zero while queue is not full");

  elements_[(head_ + size_) % capacity_] = element;
  ++size_;
  return true;
}

template <class T>
inline void RingQueue<T>::Push(T&& element) {
  COMET_FASSERT(TryPush(std::move(element)), true, "RingQueue::Push",
                "queue is full", "capacity", capacity_);
}

template <class T>
inline void RingQueue<T>::Push(const T& element) {
  COMET_FASSERT(TryPush(element), true, "RingQueue::Push", "queue is full",
                "capacity", capacity_);
}

template <class T>
inline T* RingQueue<T>::TryGet() noexcept {
  if (size_ == 0) {
    return nullptr;
  }

  return &elements_[head_];
}

template <class T>
inline const T* RingQueue<T>::TryGet() const noexcept {
  if (size_ == 0) {
    return nullptr;
  }

  return &elements_[head_];
}

template <class T>
inline T& RingQueue<T>::Get() {
  auto* element{TryGet()};
  COMET_ASSERT(element != nullptr, "RingQueue::Get", "queue is empty");
  return *element;
}

template <class T>
inline const T& RingQueue<T>::Get() const {
  const auto* element{TryGet()};
  COMET_ASSERT(element != nullptr, "RingQueue::Get", "queue is empty");
  return *element;
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

  bool TryPush(T&& element);
  bool TryPush(const T& element);
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

  struct alignas(thread::kCacheLineSize) ConsumerHead {
    std::atomic<usize> value{0};
    u8 pad[thread::kCacheLineSize - sizeof(std::atomic<usize>)]{};
  };

  struct alignas(thread::kCacheLineSize) ProducerReserve {
    std::atomic<usize> value{0};
    u8 pad[thread::kCacheLineSize - sizeof(std::atomic<usize>)]{};
  };

  void DestroyElements() noexcept;

  usize capacity_{0};
  memory::Allocator* allocator_{nullptr};
  Slot* elements_{nullptr};

  COMET_DISABLE_PADDING_WARNING_BEGIN
  ConsumerHead consumer_head_{};
  ProducerReserve producer_reserve_{};
  COMET_DISABLE_PADDING_WARNING_END
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
}

template <class T>
inline LockFreeMPSCRingQueue<T>::LockFreeMPSCRingQueue(
    LockFreeMPSCRingQueue<T>&& other) noexcept
    : capacity_{other.capacity_},
      allocator_{other.allocator_},
      elements_{other.elements_} {
  consumer_head_.value.store(
      other.consumer_head_.value.load(std::memory_order_acquire),
      std::memory_order_relaxed);
  producer_reserve_.value.store(
      other.producer_reserve_.value.load(std::memory_order_acquire),
      std::memory_order_relaxed);

  other.capacity_ = 0;
  other.allocator_ = nullptr;
  other.elements_ = nullptr;
  other.consumer_head_.value.store(0, std::memory_order_relaxed);
  other.producer_reserve_.value.store(0, std::memory_order_relaxed);
}

template <class T>
inline LockFreeMPSCRingQueue<T>& LockFreeMPSCRingQueue<T>::operator=(
    LockFreeMPSCRingQueue<T>&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Destroy();

  capacity_ = other.capacity_;
  allocator_ = other.allocator_;
  elements_ = other.elements_;

  consumer_head_.value.store(
      other.consumer_head_.value.load(std::memory_order_acquire),
      std::memory_order_relaxed);
  producer_reserve_.value.store(
      other.producer_reserve_.value.load(std::memory_order_acquire),
      std::memory_order_relaxed);

  other.capacity_ = 0;
  other.allocator_ = nullptr;
  other.elements_ = nullptr;
  other.consumer_head_.value.store(0, std::memory_order_relaxed);
  other.producer_reserve_.value.store(0, std::memory_order_relaxed);
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
  allocator_ = nullptr;
  consumer_head_.value.store(0, std::memory_order_relaxed);
  producer_reserve_.value.store(0, std::memory_order_relaxed);
}

template <class T>
inline bool LockFreeMPSCRingQueue<T>::TryPush(const T& element) {
  auto pos{producer_reserve_.value.load(std::memory_order_relaxed)};

  for (;;) {
    auto& slot{elements_[pos % capacity_]};
    const auto seq{slot.sequence.load(std::memory_order_acquire)};
    const auto delta{static_cast<sptrdiff>(seq) - static_cast<sptrdiff>(pos)};

    if (delta == 0) {
      if (producer_reserve_.value.compare_exchange_weak(
              pos, pos + 1, std::memory_order_acq_rel,
              std::memory_order_relaxed)) {
        slot.element = element;
        slot.sequence.store(pos + 1, std::memory_order_release);
        return true;
      }

      continue;
    }

    if (delta < 0) {
      return false;
    }

    pos = producer_reserve_.value.load(std::memory_order_relaxed);
  }
}

template <class T>
inline bool LockFreeMPSCRingQueue<T>::TryPush(T&& element) {
  auto pos{producer_reserve_.value.load(std::memory_order_relaxed)};

  for (;;) {
    auto& slot{elements_[pos % capacity_]};
    const auto seq{slot.sequence.load(std::memory_order_acquire)};
    const auto delta{static_cast<sptrdiff>(seq) - static_cast<sptrdiff>(pos)};

    if (delta == 0) {
      if (producer_reserve_.value.compare_exchange_weak(
              pos, pos + 1, std::memory_order_acq_rel,
              std::memory_order_relaxed)) {
        slot.element = std::move(element);
        slot.sequence.store(pos + 1, std::memory_order_release);
        return true;
      }

      continue;
    }

    if (delta < 0) {
      return false;
    }

    pos = producer_reserve_.value.load(std::memory_order_relaxed);
  }
}

template <class T>
inline void LockFreeMPSCRingQueue<T>::Push(const T& element) {
  COMET_FASSERT(TryPush(element), true, "LockFreeMPSCRingQueue::Push",
                "queue is full", "capacity", capacity_);
}

template <class T>
inline void LockFreeMPSCRingQueue<T>::Push(T&& element) {
  COMET_FASSERT(TryPush(std::move(element)), true,
                "LockFreeMPSCRingQueue::Push", "queue is full", "capacity",
                capacity_);
}

template <class T>
inline bool LockFreeMPSCRingQueue<T>::TryPop(T& element) {
  const auto pos{consumer_head_.value.load(std::memory_order_relaxed)};
  auto& slot{elements_[pos % capacity_]};

  const auto seq{slot.sequence.load(std::memory_order_acquire)};
  const auto delta{static_cast<sptrdiff>(seq) - static_cast<sptrdiff>(pos + 1)};

  if (delta != 0) {
    return false;
  }

  element = std::move(slot.element);
  slot.sequence.store(pos + capacity_, std::memory_order_release);
  consumer_head_.value.store(pos + 1, std::memory_order_relaxed);
  return true;
}

template <class T>
inline void LockFreeMPSCRingQueue<T>::Clear() {
  T element{};
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

  bool TryPush(const T& element);
  bool TryPush(T&& element);
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

  struct alignas(thread::kCacheLineSize) ProducerHead {
    std::atomic<usize> value{0};
    u8 pad[thread::kCacheLineSize - sizeof(std::atomic<usize>)]{};
  };

  struct alignas(thread::kCacheLineSize) ConsumerTail {
    std::atomic<usize> value{0};
    u8 pad[thread::kCacheLineSize - sizeof(std::atomic<usize>)]{};
  };

  void DestroyElements() noexcept;

  usize mask_{0};
  Node* elements_{nullptr};
  usize capacity_{0};
  memory::Allocator* allocator_{nullptr};

  COMET_DISABLE_PADDING_WARNING_BEGIN
  ProducerHead producer_head_{};
  ConsumerTail consumer_tail_{};
  COMET_DISABLE_PADDING_WARNING_END
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
}

template <class T>
inline LockFreeMPMCRingQueue<T>::LockFreeMPMCRingQueue(
    LockFreeMPMCRingQueue<T>&& other) noexcept
    : mask_{other.mask_},
      elements_{other.elements_},
      capacity_{other.capacity_},
      allocator_{other.allocator_} {
  producer_head_.value.store(
      other.producer_head_.value.load(std::memory_order_acquire),
      std::memory_order_relaxed);
  consumer_tail_.value.store(
      other.consumer_tail_.value.load(std::memory_order_acquire),
      std::memory_order_relaxed);

  other.mask_ = 0;
  other.elements_ = nullptr;
  other.capacity_ = 0;
  other.allocator_ = nullptr;
  other.producer_head_.value.store(0, std::memory_order_relaxed);
  other.consumer_tail_.value.store(0, std::memory_order_relaxed);
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
  capacity_ = other.capacity_;
  allocator_ = other.allocator_;

  producer_head_.value.store(
      other.producer_head_.value.load(std::memory_order_acquire),
      std::memory_order_relaxed);
  consumer_tail_.value.store(
      other.consumer_tail_.value.load(std::memory_order_acquire),
      std::memory_order_relaxed);

  other.mask_ = 0;
  other.elements_ = nullptr;
  other.capacity_ = 0;
  other.allocator_ = nullptr;
  other.producer_head_.value.store(0, std::memory_order_relaxed);
  other.consumer_tail_.value.store(0, std::memory_order_relaxed);
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
  allocator_ = nullptr;
  producer_head_.value.store(0, std::memory_order_relaxed);
  consumer_tail_.value.store(0, std::memory_order_relaxed);
}

template <class T>
inline bool LockFreeMPMCRingQueue<T>::TryPush(const T& element) {
  Node* node{nullptr};
  auto pos{producer_head_.value.load(std::memory_order_relaxed)};

  for (;;) {
    node = &elements_[pos & mask_];
    const auto seq{node->sequence.load(std::memory_order_acquire)};
    const auto delta{static_cast<sptrdiff>(seq) - static_cast<sptrdiff>(pos)};

    if (delta == 0) {
      if (producer_head_.value.compare_exchange_weak(
              pos, pos + 1, std::memory_order_relaxed,
              std::memory_order_relaxed)) {
        node->element = element;
        node->sequence.store(pos + 1, std::memory_order_release);
        return true;
      }

      continue;
    }

    if (delta < 0) {
      return false;
    }

    pos = producer_head_.value.load(std::memory_order_relaxed);
  }
}

template <class T>
inline bool LockFreeMPMCRingQueue<T>::TryPush(T&& element) {
  Node* node{nullptr};
  auto pos{producer_head_.value.load(std::memory_order_relaxed)};

  for (;;) {
    node = &elements_[pos & mask_];
    const auto seq{node->sequence.load(std::memory_order_acquire)};
    const auto delta{static_cast<sptrdiff>(seq) - static_cast<sptrdiff>(pos)};

    if (delta == 0) {
      if (producer_head_.value.compare_exchange_weak(
              pos, pos + 1, std::memory_order_relaxed,
              std::memory_order_relaxed)) {
        node->element = std::move(element);
        node->sequence.store(pos + 1, std::memory_order_release);
        return true;
      }

      continue;
    }

    if (delta < 0) {
      return false;
    }

    pos = producer_head_.value.load(std::memory_order_relaxed);
  }
}

template <class T>
inline void LockFreeMPMCRingQueue<T>::Push(const T& element) {
  COMET_FASSERT(TryPush(element), true, "LockFreeMPMCRingQueue::Push",
                "queue is full", "capacity", capacity_);
}

template <class T>
inline void LockFreeMPMCRingQueue<T>::Push(T&& element) {
  COMET_FASSERT(TryPush(std::move(element)), true,
                "LockFreeMPMCRingQueue::Push", "queue is full", "capacity",
                capacity_);
}

template <class T>
inline std::optional<T> LockFreeMPMCRingQueue<T>::TryPop() {
  Node* node;
  auto pos{consumer_tail_.value.load(std::memory_order_relaxed)};

  for (;;) {
    node = &elements_[pos & mask_];

    const auto seq{node->sequence.load(std::memory_order_acquire)};
    const auto delta{static_cast<sptrdiff>(seq) -
                     static_cast<sptrdiff>(pos + 1)};

    if (delta == 0) {
      if (consumer_tail_.value.compare_exchange_weak(
              pos, pos + 1, std::memory_order_relaxed,
              std::memory_order_relaxed)) {
        break;
      }
    } else if (delta < 0) {
      return std::nullopt;
    } else {
      pos = consumer_tail_.value.load(std::memory_order_relaxed);
    }
  }

  auto element{std::move(node->element)};
  node->sequence.store(pos + capacity_, std::memory_order_release);
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

#endif  // COMET_CORE_CONTAINER_RING_QUEUE_H_