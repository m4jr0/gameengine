// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_PRIMITIVE_H_
#define COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_PRIMITIVE_H_

#include <atomic>
#include <deque>

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/concurrency/fiber/fiber_context.h"
#include "comet/core/essentials.h"

namespace comet {
namespace fiber {
#ifdef COMET_DEBUG
using FiberPrimitiveDebugId = usize;
constexpr auto kInvalidFiberPrimitiveDebugId{
    static_cast<FiberPrimitiveDebugId>(-1)};
#endif  // COMET_DEBUG

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
#ifdef COMET_DEBUG
  static_assert(std::atomic<FiberPrimitiveDebugId>::is_always_lock_free,
                "std::atomic<FiberPrimitiveDebugId> needs to be always "
                "lock-free. Unsupported "
                "architecture");
  static inline std::atomic<FiberPrimitiveDebugId> id_counter_{0};
  FiberPrimitiveDebugId id_{id_counter_++};
#endif  // COMET_DEBUG
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
#ifdef COMET_DEBUG
  static_assert(std::atomic<FiberPrimitiveDebugId>::is_always_lock_free,
                "std::atomic<FiberPrimitiveDebugId> needs to be always "
                "lock-free. Unsupported "
                "architecture");
  static inline std::atomic<FiberPrimitiveDebugId> id_counter_{0};
  FiberPrimitiveDebugId id_{id_counter_++};
#endif  // COMET_DEBUG
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
#ifdef COMET_DEBUG
  static_assert(std::atomic<FiberPrimitiveDebugId>::is_always_lock_free,
                "std::atomic<FiberPrimitiveDebugId> needs to be always "
                "lock-free. Unsupported "
                "architecture");
  static inline std::atomic<FiberPrimitiveDebugId> id_counter_{0};
  FiberPrimitiveDebugId id_{id_counter_++};
#endif  // COMET_DEBUG
  FiberSpinLock spin_lock_{};
  Fiber* owner_{nullptr};
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
#ifdef COMET_DEBUG
  static_assert(std::atomic<FiberPrimitiveDebugId>::is_always_lock_free,
                "std::atomic<FiberPrimitiveDebugId> needs to be always "
                "lock-free. Unsupported "
                "architecture");
  static inline std::atomic<FiberPrimitiveDebugId> id_counter_{0};
  FiberPrimitiveDebugId id_{id_counter_++};
#endif  // COMET_DEBUG
  FiberMutex& mutex_;
};

class FiberAwareLockGuard {
 public:
  FiberAwareLockGuard() = delete;
  explicit FiberAwareLockGuard(SimpleLock& lock);
  FiberAwareLockGuard(const FiberAwareLockGuard&) = delete;
  FiberAwareLockGuard(FiberAwareLockGuard&&) = delete;
  FiberAwareLockGuard& operator=(const FiberAwareLockGuard&) = delete;
  FiberAwareLockGuard& operator=(FiberAwareLockGuard&&) = delete;
  ~FiberAwareLockGuard();

 private:
#ifdef COMET_DEBUG
  static_assert(std::atomic<FiberPrimitiveDebugId>::is_always_lock_free,
                "std::atomic<FiberPrimitiveDebugId> needs to be always "
                "lock-free. Unsupported "
                "architecture");
  static inline std::atomic<FiberPrimitiveDebugId> id_counter_{0};
  FiberPrimitiveDebugId id_{id_counter_++};
#endif  // COMET_DEBUG
  SimpleLock& lock_;
};

class FiberUniqueLock {
 public:
  FiberUniqueLock() = delete;
  explicit FiberUniqueLock(FiberMutex& mtx, bool is_lock_deferred = false);
  FiberUniqueLock(const FiberUniqueLock&) = delete;
  FiberUniqueLock(FiberUniqueLock&&) noexcept = delete;
  FiberUniqueLock& operator=(const FiberUniqueLock&) = delete;
  FiberUniqueLock& operator=(FiberUniqueLock&&) noexcept = delete;
  ~FiberUniqueLock();

  void Lock();
  void Unlock();

 private:
  FiberMutex& mtx_;
  bool is_mutex_owned_{false};
};

class FiberCV {
 public:
  FiberCV() = default;
  FiberCV(const FiberCV&) = delete;
  FiberCV(FiberCV&&) = delete;
  FiberCV& operator=(const FiberCV&) = delete;
  FiberCV& operator=(FiberCV&&) = delete;
  ~FiberCV() = default;

  void Wait(FiberUniqueLock& lock);

  template <typename Predicate>
  void Wait(FiberUniqueLock& lock, Predicate&& pred) {
    COMET_ASSERT(IsFiber(), "Current thread is not a fiber!");
    auto* fiber{GetFiber()};
    COMET_ASSERT(fiber != nullptr, "Current fiber is null!");

    {
      FiberSpinLockGuard spin_lock{spin_lock_};

      if (!pred()) {
        awaiting_fibers_.push_back(fiber);
      }
    }

    while (!pred()) {
      lock.Unlock();
      Yield();
      lock.Lock();
    }
  }

  void NotifyOne();
  void NotifyAll();

 private:
#ifdef COMET_DEBUG
  static_assert(std::atomic<FiberPrimitiveDebugId>::is_always_lock_free,
                "std::atomic<FiberPrimitiveDebugId> needs to be always "
                "lock-free. Unsupported "
                "architecture");
  static inline std::atomic<FiberPrimitiveDebugId> id_counter_{0};
  FiberPrimitiveDebugId id_{id_counter_++};
#endif  // COMET_DEBUG
  FiberSpinLock spin_lock_{};
  std::deque<Fiber*> awaiting_fibers_{};
};

class FiberSharedMutex {
 public:
  FiberSharedMutex() = default;
  FiberSharedMutex(const FiberSharedMutex&) = delete;
  FiberSharedMutex(FiberSharedMutex&&) = delete;
  FiberSharedMutex& operator=(const FiberSharedMutex&) = delete;
  FiberSharedMutex& operator=(FiberSharedMutex&&) = delete;
  ~FiberSharedMutex() = default;

  void LockExclusive();
  void UnlockExclusive();

  void LockShared();
  void UnlockShared();

 private:
  FiberMutex mutex_{};
  FiberCV cv_{};
  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> needs to be always lock-free. Unsupported "
                "architecture");
  std::atomic<usize> reader_count_{0};
  bool is_writer_{false};
};

enum class FiberSharedLockType { Unknown = 0, Shared, Exclusive };

class FiberSharedLockGuard {
 public:
  FiberSharedLockGuard(FiberSharedMutex& mutex, FiberSharedLockType type);
  FiberSharedLockGuard(const FiberSharedLockGuard&) = delete;
  FiberSharedLockGuard(FiberSharedLockGuard&&) = delete;
  FiberSharedLockGuard& operator=(const FiberSharedLockGuard&) = delete;
  FiberSharedLockGuard& operator=(FiberSharedLockGuard&&) = delete;
  ~FiberSharedLockGuard();

 private:
  FiberSharedMutex& mutex_;
  FiberSharedLockType type_{FiberSharedLockType::Unknown};
};
}  // namespace fiber
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_PRIMITIVE_H_