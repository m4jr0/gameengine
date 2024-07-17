// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "fiber_primitive.h"

#include <string>  // >:3

#include "comet/core/debug.h"
#include "comet/core/job/fiber/fiber_context.h"
#include "comet/core/job/worker.h"  // >:3

namespace comet {
namespace job {
void FiberSpinLock::Lock() {
  while (flag_.test_and_set(std::memory_order_acquire)) {
    Yield();
  }

  Worker::DumpData("Lock #" + std::to_string(id_) + " SpinLock Locked!");
}

bool FiberSpinLock::TryLock() {
  return flag_.test_and_set(std::memory_order_acquire);
}

void FiberSpinLock::Unlock() {
  Worker::DumpData("Lock #" + std::to_string(id_) + " SpinLock Unlocked!");
  flag_.clear(std::memory_order_release);
}

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
      Worker::DumpData("Lock #" + std::to_string(id_) + " Becoming owner!");

      if (GetCurrent()->GetId() == 128) {
        int a = 0;
        ++a;
      }

      owner_ = fiber;
      return;
    }

    Worker::DumpData("Lock #" + std::to_string(id_) +
                     " Awaiting to become owner!");
    Yield();
  }
}

void FiberMutex::WaitForLock() {
  while (spin_lock_.TryLock())
    ;
}

void FiberMutex::Unlock() {
  auto* fiber{GetCurrent()};
  FiberSpinLockGuard guard{spin_lock_};
  COMET_ASSERT(fiber == owner_, "Lock is not owned by current fiber!");
  Worker::DumpData("Lock #" + std::to_string(id_) +
                   "  Giving ownership back...");
  owner_ = nullptr;
}

void FiberCV::Wait(FiberMutex& mtx) {
  mtx.Unlock();

  mtx.Lock();
}

void FiberCV::NotifyOne() {}

void FiberCV::NotifyAll() {}

FiberLock::FiberLock(FiberMutex& mutex, bool is_blocking) : mutex_{mutex} {
  if (!is_blocking) {
    mutex_.Lock();
  } else {
    mutex_.WaitForLock();
  }
}

FiberLock::~FiberLock() { mutex_.Unlock(); }
}  // namespace job
}  // namespace comet