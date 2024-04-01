// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_JOB_WORKER_H_
#define COMET_COMET_CORE_CONCURRENCY_JOB_WORKER_H_

#include <thread>

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/essentials.h"

namespace comet {
namespace job {
using WorkerId = usize;
constexpr auto kInvalidWorkerId{static_cast<WorkerId>(-1)};

class Worker {
 public:
  static Worker& GetCurrent();

  Worker() = delete;
  explicit Worker(std::thread&& thread);
  Worker(const Worker&) = delete;
  Worker(Worker&& other) noexcept;

  Worker& operator=(const Worker&) = delete;
  Worker& operator=(Worker&& other) noexcept;
  ~Worker() = default;

  void Initialize();
  void Destroy();

  WorkerId GetId() const noexcept;

 private:
  inline static WorkerId id_counter_{0};

  WorkerId id_{kInvalidWorkerId};
  std::thread thread_{};
  // Default constructed to represent current thread as a fiber.
  fiber::Fiber* worker_fiber_{nullptr};
  fiber::Fiber* current_fiber_{nullptr};
};

class IOWorker {
 public:
  IOWorker() = delete;
  explicit IOWorker(std::thread&& thread);
  IOWorker(const IOWorker&) = delete;
  IOWorker(IOWorker&& other) noexcept;
  IOWorker& operator=(const IOWorker&) = delete;
  IOWorker& operator=(IOWorker&& other) noexcept;
  ~IOWorker() = default;

  void Initialize();
  void Destroy();

  WorkerId GetId() const noexcept;

 private:
  inline static WorkerId id_counter_{0};

  WorkerId id_{kInvalidWorkerId};
  std::thread thread_{};
  // Default constructed to represent current thread as a fiber.
  fiber::Fiber* worker_fiber_{nullptr};
  fiber::Fiber* current_fiber_{nullptr};
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_JOB_WORKER_H_