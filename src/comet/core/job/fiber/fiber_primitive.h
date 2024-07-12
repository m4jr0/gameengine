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
  std::atomic_flag flag_{ATOMIC_FLAG_INIT};
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

class FiberLock {
 public:
  FiberLock() = delete;
  explicit FiberLock(FiberMutex& mutex);
  FiberLock(const FiberLock&) = delete;
  FiberLock(FiberLock&&) = delete;
  FiberLock& operator=(const FiberLock&) = delete;
  FiberLock& operator=(FiberLock&&) = delete;
  ~FiberLock();

 private:
  static inline FiberPrimitiveId id_counter_{0};
  FiberPrimitiveId id_{id_counter_++};
  FiberMutex& mutex_;
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