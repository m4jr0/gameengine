// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "fiber_primitive.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_context.h"

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
    FiberSpinLockGuard guard{spin_lock_};
    COMET_ASSERT(fiber != owner_, "Lock is already owned by current fiber!");

    if (owner_ == nullptr) {
      owner_ = fiber;
      return;
    }

    Yield();
  }
}

void FiberMutex::Unlock() {
  [[maybe_unused]] auto* fiber{GetFiber()};
  FiberSpinLockGuard guard{spin_lock_};
  COMET_ASSERT(fiber == owner_, "Lock is not owned by current fiber!");
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
  COMET_ASSERT(fiber != nullptr, "Current fiber is null!");

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

    if (to_resume != nullptr) {
      internal::Sleep(to_resume);
    }
  }
}

void FiberCV::NotifyAll() {
  std::deque<Fiber*> to_resume{};

  {
    FiberSpinLockGuard lock{spin_lock_};
    to_resume.swap(awaiting_fibers_);
  }

  for (auto* fiber : to_resume) {
    internal::SleepTo(fiber);
  }
}

void FiberSharedMutex::LockExclusive() {
  FiberUniqueLock lock{mutex_};

  while (reader_count_.load(std::memory_order_acquire) > 0) {
    cv_.Wait(lock);
  }

  is_writer_ = true;
}

void FiberSharedMutex::UnlockExclusive() {
  FiberLockGuard lock{mutex_};
  is_writer_ = false;
  cv_.NotifyAll();
}

void FiberSharedMutex::LockShared() {
  FiberUniqueLock lock{mutex_};
  cv_.Wait(lock, [this] { return !is_writer_; });
  reader_count_.fetch_add(std::memory_order_acq_rel);
}

void FiberSharedMutex::UnlockShared() {
  FiberLockGuard lock{mutex_};

  if (reader_count_.fetch_sub(std::memory_order_acq_rel) == 1) {
    cv_.NotifyAll();
  }
}

FiberSharedLockGuard::FiberSharedLockGuard(FiberSharedMutex& mutex,
                                           FiberSharedLockType type)
    : mutex_{mutex}, type_{type} {
  COMET_ASSERT(type_ == FiberSharedLockType::Exclusive ||
                   type_ == FiberSharedLockType::Shared,
               "Unknown or unsupported lock type provided: ",
               static_cast<std::underlying_type_t<FiberSharedLockType>>(type_),
               "!");

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