// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_JOB_WORKER_H_
#define COMET_COMET_CORE_CONCURRENCY_JOB_WORKER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/concurrency/thread/thread.h"
#include "comet/core/essentials.h"

namespace comet {
namespace job {
using WorkerId = usize;
constexpr auto kInvalidWorkerId{static_cast<WorkerId>(-1)};

using WorkerTypeIndex = u32;
constexpr auto kInvalidWorkerTypeIndex{static_cast<WorkerTypeIndex>(-1)};

using WorkerTag = u32;
constexpr auto kInvalidWorkerTag{static_cast<WorkerTag>(-1)};

class Worker {
 public:
  ~Worker() = default;

  virtual void Attach();
  virtual void Detach();

  template <typename ThreadFunc, typename... Targs>
  void Run(ThreadFunc&& func, Targs&&... args) {
    thread_.Run(std::forward<ThreadFunc>(func), std::forward<Targs>(args)...);
  }

  void Stop();

  WorkerId GetId() const noexcept;
  virtual WorkerTag GetTag() const noexcept;
  virtual WorkerTypeIndex GetTypeIndex() const noexcept;

 protected:
  Worker(WorkerTag tag, WorkerTypeIndex type_index);
  Worker(const Worker&) = delete;
  Worker(Worker&& other) noexcept;

  Worker& operator=(const Worker&) = delete;
  Worker& operator=(Worker&& other) noexcept;

 private:
  WorkerTag tag_{kInvalidWorkerTag};
  WorkerTypeIndex type_index_{kInvalidWorkerTypeIndex};
  thread::Thread thread_{};
};

class FiberWorker : public Worker {
 public:
  inline static constexpr WorkerTag kTag_{1};

  FiberWorker();
  FiberWorker(const FiberWorker&) = delete;
  FiberWorker(FiberWorker&& other) noexcept;
  FiberWorker& operator=(const FiberWorker&) = delete;
  FiberWorker& operator=(FiberWorker&& other) noexcept;
  ~FiberWorker() = default;

  void Attach() override;
  void Detach() override;

 private:
  static_assert(
      std::atomic<WorkerTypeIndex>::is_always_lock_free,
      "std::atomic<WorkerIndex> needs to be always lock-free. Unsupported "
      "architecture");

  inline static std::atomic<WorkerTypeIndex> index_counter_{0};

  fiber::Fiber* worker_fiber_{nullptr};
  fiber::Fiber* current_fiber_{nullptr};
};

class IOWorker : public Worker {
 public:
  inline static constexpr WorkerTag kTag_{2};

  IOWorker();
  IOWorker(const IOWorker&) = delete;
  IOWorker(IOWorker&& other) noexcept;
  IOWorker& operator=(const IOWorker&) = delete;
  IOWorker& operator=(IOWorker&& other) noexcept;
  ~IOWorker() = default;

  void Attach() override;
  void Detach() override;

 private:
  static_assert(
      std::atomic<WorkerTypeIndex>::is_always_lock_free,
      "std::atomic<WorkerIndex> needs to be always lock-free. Unsupported "
      "architecture");

  inline static std::atomic<WorkerTypeIndex> index_counter_{0};
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_JOB_WORKER_H_