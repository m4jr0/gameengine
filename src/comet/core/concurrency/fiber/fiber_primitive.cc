// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "fiber_primitive.h"

#include "comet/core/concurrency/fiber/fiber_context.h"

namespace comet {
namespace fiber {
void SimpleLock::Lock() {
  while (!TryLock())
    ;
}

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
    auto* fiber{GetCurrent()};
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
  auto* fiber{GetCurrent()};
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

void FiberCV::Wait(FiberMutex& mtx) {
  mtx.Unlock();

  mtx.Lock();
}

void FiberCV::NotifyOne() {}

void FiberCV::NotifyAll() {}
}  // namespace fiber
}  // namespace comet