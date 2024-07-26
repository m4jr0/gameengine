// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_JOB_FIBER_PRIMITIVE_H_
#define COMET_COMET_CORE_JOB_FIBER_PRIMITIVE_H_

#include <atomic>
#include <deque>

#include "comet/core/job/fiber/fiber.h"
#include "comet/core/type/primitive.h"

namespace comet {
namespace job {
using FiberPrimitiveId = uindex;
constexpr auto kInvalidFiberPrimitiveId{static_cast<FiberPrimitiveId>(-1)};

class SimpleLock {
 public:
  SimpleLock() = default;
  SimpleLock(const SimpleLock&) = delete;
  SimpleLock(SimpleLock&&) = delete;
  SimpleLock& operator=(const SimpleLock&) = delete;
  SimpleLock& operator=(SimpleLock&&) = delete;
  ~SimpleLock() = default;

  void Lock();
  bool TryLock();
  void Unlock();

 private:
  std::atomic_flag flag_{ATOMIC_FLAG_INIT};
};

class SimpleLockGuard {
 public:
  SimpleLockGuard() = delete;
  explicit SimpleLockGuard(SimpleLock& lock);
  SimpleLockGuard(const SimpleLockGuard&) = delete;
  SimpleLockGuard(SimpleLockGuard&&) = delete;
  SimpleLockGuard& operator=(const SimpleLockGuard&) = delete;
  SimpleLockGuard& operator=(SimpleLockGuard&&) = delete;
  ~SimpleLockGuard();

 private:
  SimpleLock& lock_;
};

class FiberSpinLock {
 public:
  FiberSpinLock() = default;
  FiberSpinLock(const FiberSpinLock&) = delete;
  FiberSpinLock(FiberSpinLock&&) = delete;
  FiberSpinLock& operator=(const FiberSpinLock&) = delete;
  FiberSpinLock& operator=(FiberSpinLock&&) = delete;
  ~FiberSpinLock() = default;

  void Lock();
  void Unlock();

 private:
  static inline FiberPrimitiveId id_counter_{0};
  FiberPrimitiveId id_{id_counter_++};
  SimpleLock lock_{};
};

class FiberSpinLockGuard {
 public:
  FiberSpinLockGuard() = delete;
  explicit FiberSpinLockGuard(FiberSpinLock& spin_lock);
  FiberSpinLockGuard(const FiberSpinLockGuard&) = delete;
  FiberSpinLockGuard(FiberSpinLockGuard&&) = delete;
  FiberSpinLockGuard& operator=(const FiberSpinLockGuard&) = delete;
  FiberSpinLockGuard& operator=(FiberSpinLockGuard&&) = delete;
  ~FiberSpinLockGuard();

 private:
  static inline FiberPrimitiveId id_counter_{0};
  FiberPrimitiveId id_{id_counter_++};
  FiberSpinLock& spin_lock_;
};

class FiberMutex {
 public:
  FiberMutex() = default;
  FiberMutex(const FiberMutex&) = delete;
  FiberMutex(FiberMutex&&) = delete;
  FiberMutex& operator=(const FiberMutex&) = delete;
  FiberMutex& operator=(FiberMutex&&) = delete;
  ~FiberMutex() = default;

  void Lock();
  void Unlock();

 private:
  static inline FiberPrimitiveId id_counter_{0};
  FiberPrimitiveId id_{id_counter_++};
  FiberSpinLock spin_lock_{};
  Fiber* owner_{nullptr};
  std::deque<Fiber*> awaiting_fibers_{};
};

class FiberLockGuard {
 public:
  FiberLockGuard() = delete;
  explicit FiberLockGuard(FiberMutex& mutex);
  FiberLockGuard(const FiberLockGuard&) = delete;
  FiberLockGuard(FiberLockGuard&&) = delete;
  FiberLockGuard& operator=(const FiberLockGuard&) = delete;
  FiberLockGuard& operator=(FiberLockGuard&&) = delete;
  ~FiberLockGuard();

 private:
  static inline FiberPrimitiveId id_counter_{0};
  FiberPrimitiveId id_{id_counter_++};
  FiberMutex& mutex_;
};

class FiberAwareLockGuard{
 public:
  FiberAwareLockGuard() = delete;
  explicit FiberAwareLockGuard(SimpleLock& lock);
  FiberAwareLockGuard(const FiberAwareLockGuard&) = delete;
  FiberAwareLockGuard(FiberAwareLockGuard&&) = delete;
  FiberAwareLockGuard& operator=(const FiberAwareLockGuard&) = delete;
  FiberAwareLockGuard& operator=(FiberAwareLockGuard&&) = delete;
  ~FiberAwareLockGuard();

 private:
  static inline FiberPrimitiveId id_counter_{0};
  FiberPrimitiveId id_{id_counter_++};
  SimpleLock& lock_;
};

class FiberCV {
 public:
  FiberCV() = default;
  FiberCV(const FiberCV&) = delete;
  FiberCV(FiberCV&&) = delete;
  FiberCV& operator=(const FiberCV&) = delete;
  FiberCV& operator=(FiberCV&&) = delete;
  ~FiberCV() = default;

  void Wait(FiberMutex& mtx);

  template <typename Predicate>
  void Wait(FiberMutex& mutex, Predicate&& pred) {
    if (pred()) {
      return;
    }

    // auto* fiber{Worker::GetCurrent()}; // >:3 Wrong function.
    // COMET_ASSERT(fiber != nullptr, "Fiber is null!");
    mutex.Lock();
  }

  void NotifyOne();
  void NotifyAll();

 private:
  static inline FiberPrimitiveId id_counter_{0};
  FiberPrimitiveId id_{id_counter_++};
  std::atomic<uindex> waiting_count_{0};
  std::atomic<uindex> waiting_on_pred_count_{0};
  std::deque<Fiber*> awaiting_fibers_{};
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_FIBER_PRIMITIVE_H_