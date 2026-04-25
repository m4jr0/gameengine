// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "fiber_primitive.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_context.h"
#include "comet/core/concurrency/fiber/fiber_label.h"
#include "comet/core/type_trait.h"

namespace comet {
namespace fiber {
void SimpleLock::Lock() { while (!TryLock()); }

bool SimpleLock::TryLock() {
  return !flag_.test_and_set(std::memory_order_acquire);
}

void SimpleLock::Unlock() { flag_.clear(std::memory_order_release); }

void FiberSpinLock::Lock() {
  while (!lock_.TryLock()) {
    Yield();
  }
}

void FiberSpinLock::Unlock() { lock_.Unlock(); }

SimpleLockGuard::SimpleLockGuard(SimpleLock& lock) : lock_{lock} {
  lock_.Lock();
}

SimpleLockGuard::~SimpleLockGuard() { lock_.Unlock(); }

FiberSpinLockGuard::FiberSpinLockGuard(FiberSpinLock& spin_lock)
    : spin_lock_{spin_lock} {
  spin_lock.Lock();
}

FiberSpinLockGuard::~FiberSpinLockGuard() { spin_lock_.Unlock(); }

void FiberMutex::Lock() {
  for (;;) {
    auto* fiber{GetFiber()};
    auto is_locked{false};

    {
      FiberSpinLockGuard guard{spin_lock_};
      COMET_ASSERT(fiber != owner_, "FiberMutex::Lock",
                   "lock is already owned by current fiber");

      if (owner_ == nullptr) {
        owner_ = fiber;
        is_locked = true;
      }
    }

    if (is_locked) {
      return;
    }

    Yield();
  }
}

void FiberMutex::Unlock() {
  [[maybe_unused]] auto* fiber{GetFiber()};
  FiberSpinLockGuard guard{spin_lock_};
  COMET_ASSERT(fiber == owner_, "FiberMutex::Unlock",
               "lock is not owned by current fiber");
  owner_ = nullptr;
}

FiberLockGuard::FiberLockGuard(FiberMutex& mutex) : mutex_{mutex} {
  mutex_.Lock();
}

FiberLockGuard::~FiberLockGuard() { mutex_.Unlock(); }

FiberAwareLockGuard::FiberAwareLockGuard(SimpleLock& lock) : lock_{lock} {
  // Case: blockable thread (no fiber is being executed).
  if (!IsFiber()) {
    lock_.Lock();
    return;
  }

  while (!lock_.TryLock()) {
    Yield();
  }
}

FiberAwareLockGuard::~FiberAwareLockGuard() { lock_.Unlock(); }

FiberUniqueLock::FiberUniqueLock(FiberMutex& mtx, bool is_lock_deferred)
    : mtx_{mtx}, is_mutex_owned_{!is_lock_deferred} {
  if (is_mutex_owned_) {
    mtx_.Lock();
  }
}

FiberUniqueLock::~FiberUniqueLock() { Unlock(); }

void FiberUniqueLock::Lock() {
  if (!is_mutex_owned_) {
    mtx_.Lock();
    is_mutex_owned_ = true;
  }
}

void FiberUniqueLock::Unlock() {
  if (is_mutex_owned_) {
    mtx_.Unlock();
    is_mutex_owned_ = false;
  }
}

void FiberCV::Wait(FiberUniqueLock& lock) {
  auto* fiber{GetFiber()};
  COMET_ASSERT(fiber != nullptr, "FiberCV::Wait", "current fiber is null");

  {
    FiberSpinLockGuard spin_lock{spin_lock_};
    awaiting_fibers_.push_back(fiber);
  }

  lock.Unlock();
  internal::ResumeWorker();
  lock.Lock();
}

void FiberCV::NotifyOne() {
  Fiber* to_resume{nullptr};

  {
    FiberSpinLockGuard lock{spin_lock_};

    if (!awaiting_fibers_.empty()) {
      to_resume = awaiting_fibers_.front();
      awaiting_fibers_.pop_front();
    }
  }

  if (to_resume != nullptr) {
    internal::Sleep(to_resume);
  }
}

void FiberCV::NotifyAll() {
  std::deque<Fiber*> to_resume{};

  {
    FiberSpinLockGuard lock{spin_lock_};
    to_resume.swap(awaiting_fibers_);
  }

  for (auto* fiber : to_resume) {
    internal::Sleep(fiber);
  }
}

void FiberSharedMutex::LockExclusive() {
  auto* fiber{GetFiber()};

  {
    FiberSpinLockGuard guard{spin_lock_};
    ++waiting_writers_;
  }

  for (;;) {
    bool acquired{false};

    {
      FiberSpinLockGuard guard{spin_lock_};

      COMET_ASSERT(writer_ != fiber, "FiberSharedMutex::Lock",
                   "exclusive lock is already owned by current fiber");

      if (writer_ == nullptr && reader_count_ == 0) {
        writer_ = fiber;
        --waiting_writers_;
        acquired = true;
      }
    }

    if (acquired) {
      return;
    }

    Yield();
  }
}

void FiberSharedMutex::UnlockExclusive() {
  [[maybe_unused]] auto* fiber{GetFiber()};

  FiberSpinLockGuard guard{spin_lock_};

  COMET_ASSERT(writer_ == fiber, "FiberSharedMutex::Unlock",
               "exclusive lock is not owned by current fiber");

  writer_ = nullptr;
}

void FiberSharedMutex::LockShared() {
  for (;;) {
    bool acquired{false};

    {
      FiberSpinLockGuard guard{spin_lock_};

      // Writer-preference: prevent new readers from starving a waiting writer.
      if (writer_ == nullptr && waiting_writers_ == 0) {
        ++reader_count_;
        acquired = true;
      }
    }

    if (acquired) {
      return;
    }

    Yield();
  }
}

void FiberSharedMutex::UnlockShared() {
  FiberSpinLockGuard guard{spin_lock_};

  COMET_ASSERT(reader_count_ > 0, "FiberSharedMutex::UnlockShared",
               "shared lock count underflow");

  --reader_count_;
}

FiberSharedLockGuard::FiberSharedLockGuard(FiberSharedMutex& mutex,
                                           FiberSharedLockType type)
    : mutex_{mutex}, type_{type} {
  COMET_ASSERT(type_ == FiberSharedLockType::Exclusive ||
                   type_ == FiberSharedLockType::Shared,
               "FiberSharedLockGuard::FiberSharedLockGuard",
               "lock type is invalid", "type",
               GetFiberSharedLockTypeLabel(type_), "type_value",
               ToUnderlying(type_));

  if (type_ == FiberSharedLockType::Exclusive) {
    mutex_.LockExclusive();
  } else {
    mutex_.LockShared();
  }
}

FiberSharedLockGuard::~FiberSharedLockGuard() {
  if (type_ == FiberSharedLockType::Exclusive) {
    mutex_.UnlockExclusive();
  } else {
    mutex_.UnlockShared();
  }
}
}  // namespace fiber
}  // namespace comet