// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "thread.h"

#include <thread>

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
  COMET_ASSERT(!IsAttached(),
               "Destructor called for thread, but it is still initialized!");
}

void Thread::TryJoin() {
  if (thread_.joinable()) {
    thread_.join();
  }
}

bool Thread::IsAttached() const noexcept {
  return thread_id_ != kInvalidThreadId;
}

ThreadId Thread::GetId() const noexcept { return thread_id_; }

void Thread::Attach() {
  COMET_ASSERT(!IsAttached(),
               "Tried to attach thread, but it is already done!");
  thread_id_ = thread_id_counter_.fetch_add(1, std::memory_order_acq_rel);
  internal::AttachThread(this);
}

void Thread::Detach() {
  COMET_ASSERT(IsAttached(), "Tried to detach thread, but it is not attached!");
  thread_id_ = kInvalidThreadId;
  internal::DetachThread();
}
}  // namespace thread
}  // namespace comet