// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_THREAD_THREAD_H_
#define COMET_COMET_CORE_CONCURRENCY_THREAD_THREAD_H_

#include <atomic>
#include <functional>
#include <thread>
#include <utility>

#include "comet/core/essentials.h"

namespace comet {
namespace thread {
using ThreadId = usize;
inline static constexpr usize kInvalidThreadId{static_cast<ThreadId>(-1)};

class Thread {
 public:
  static void AttachMainThread();
  static void DetachMainThread();

  Thread() = default;
  Thread(const Thread&) = delete;
  Thread(Thread&& other) noexcept;
  Thread& operator=(const Thread&) = delete;
  Thread& operator=(Thread&& other) noexcept;
  ~Thread();

  template <typename ThreadFunc, typename... Targs>
  void Run(ThreadFunc&& func, Targs&&... args) {
    COMET_ASSERT(!IsAttached(),
                 "Tried to run thread, but it is already attached!");
    COMET_ASSERT(!is_started_.load(std::memory_order_acquire),
                 "Tried to run thread, but it is already running!");
    COMET_ASSERT(
        thread_id_counter_.load(std::memory_order_acquire) > kMainThreadId_,
        "Main thread has not been attached!");

    auto thread_func{
        [this, func = std::forward<ThreadFunc>(func),
         argsTuple = std::make_tuple(std::forward<Targs>(args)...)]() mutable {
          Attach();
          is_started_.store(true, std::memory_order_release);
          std::apply(func, std::move(argsTuple));
          is_started_.store(false, std::memory_order_release);
          Detach();
        }};

    thread_ = std::thread(std::move(thread_func));
  }

  void TryJoin();
  bool IsAttached() const noexcept;
  ThreadId GetId() const noexcept;

 private:
  void Attach();
  void Detach();

  inline static constexpr ThreadId kMainThreadId_{0};

  static_assert(
      std::atomic<ThreadId>::is_always_lock_free,
      "std::atomic<ThreadId> needs to be always lock-free. Unsupported "
      "architecture");
  inline static std::atomic<ThreadId> thread_id_counter_{0};
  static Thread main_thread_;

  ThreadId thread_id_{kInvalidThreadId};

  static_assert(std::atomic<bool>::is_always_lock_free,
                "std::atomic<bool> needs to be always lock-free. Unsupported "
                "architecture");
  std::atomic<bool> is_started_{false};

  std::thread thread_{};
};
}  // namespace thread
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_THREAD_THREAD_H_