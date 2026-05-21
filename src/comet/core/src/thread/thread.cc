// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/thread/thread.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <thread>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/thread/thread_context.h"

namespace comet {
namespace thread {
Thread Thread::main_thread_{};

void Thread::AttachMainThread() { main_thread_.Attach(); }

void Thread::DetachMainThread() { main_thread_.Detach(); }

Thread::Thread(Thread&& other) noexcept
    : thread_id_{other.thread_id_},
      is_started_{other.is_started_.load(std::memory_order_acquire)},
      thread_{std::move(other.thread_)} {
  other.thread_id_ = kInvalidThreadId;
  other.is_started_.store(false, std::memory_order_release);
}

Thread& Thread::operator=(Thread&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  TryJoin();

  thread_id_ = other.thread_id_;
  is_started_.store(other.is_started_.load(std::memory_order_acquire),
                    std::memory_order_release);
  thread_ = std::move(other.thread_);

  other.thread_id_ = kInvalidThreadId;
  other.is_started_.store(false, std::memory_order_release);
  return *this;
}

Thread::~Thread() {
  COMET_ASSERT(!IsAttached(), "thread::Thread::~Thread",
               "thread is still attached");
}

void Thread::TryJoin() {
  if (thread_.joinable()) {
    thread_.join();
  }
}

ThreadId Thread::GetId() const noexcept { return thread_id_; }

bool Thread::IsAttached() const noexcept {
  return thread_id_ != kInvalidThreadId;
}

bool Thread::IsMain() const noexcept {
  return thread_id_ == main_thread_.thread_id_;
}

void Thread::Attach() {
  COMET_ASSERT(!IsAttached(), "thread::Thread::Attach",
               "thread is already attached");
  thread_id_ = thread_id_counter_.fetch_add(1, std::memory_order_acq_rel);
  internal::AttachThread(this);
}

void Thread::Detach() {
  COMET_ASSERT(IsAttached(), "thread::Thread::Detach",
               "thread is not attached");
  thread_id_ = kInvalidThreadId;
  internal::DetachThread();
}
}  // namespace thread
}  // namespace comet