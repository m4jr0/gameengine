// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_JOB_SCHEDULER_H_
#define COMET_COMET_CORE_JOB_SCHEDULER_H_

#include <atomic>

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/worker.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/ring_queue.h"

namespace comet {
namespace job {
namespace internal {
class FiberPool {
 public:
  FiberPool() = delete;
  FiberPool(usize fiber_count, usize fiber_stack_size);
  FiberPool(const FiberPool&) = delete;
  FiberPool(FiberPool&&) = delete;
  FiberPool& operator=(const FiberPool&) = delete;
  FiberPool& operator=(FiberPool&&) = delete;
  ~FiberPool() = default;

  void Initialize();
  void Destroy();

  fiber::Fiber* TryPop();
  void Push(fiber::Fiber* fiber);

  usize GetTotalAllocatedStackSize() const;

 private:
  usize initial_fiber_count_{0};
  usize fiber_stack_size_{0};
  memory::PlatformAllocator queue_allocator_{memory::kEngineMemoryTagFiber};
  memory::PlatformStackAllocator fiber_allocator_{};
  LockFreeMPMCRingQueue<fiber::Fiber*> fibers_{};
};

class CounterPool {
 public:
  CounterPool() = default;
  CounterPool(const CounterPool&) = delete;
  CounterPool(CounterPool&&) = delete;
  CounterPool& operator=(const CounterPool&) = delete;
  CounterPool& operator=(CounterPool&&) = delete;
  ~CounterPool() = default;

  void Initialize();
  void Destroy();

  Counter* TryGet();
  void Push(Counter* counter);

 private:
  memory::PlatformAllocator queue_allocator_{memory::kEngineMemoryTagFiber};
  memory::PlatformStackAllocator counter_allocator_{};
  LockFreeMPMCRingQueue<Counter*> counters_{};
};
}  // namespace internal

class Scheduler {
 public:
  static Scheduler& Get();

  Scheduler() = default;
  Scheduler(const Scheduler&) = delete;
  Scheduler(Scheduler&&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;
  Scheduler& operator=(Scheduler&&) = delete;
  ~Scheduler() = default;

  void Initialize();
  void Shutdown();
  void Run(const JobDescr& callback_descr);
  void RequestShutdown();

  Counter* GenerateCounter();
  void DestroyCounter(Counter* counter);

  void Kick(const JobDescr& job_descr);
  void Kick(usize job_count, const JobDescr* job_descrs);
  void Kick(const IOJobDescr& job_descr);
  void Kick(usize job_count, const IOJobDescr* job_descrs);

  void Wait(Counter* counter);

  void KickAndWait(const JobDescr& job_descr);
  void KickAndWait(usize job_count, const JobDescr* job_descrs);
  void KickAndWait(const IOJobDescr& job_descr);
  void KickAndWait(usize job_count, const IOJobDescr* job_descrs);

  usize GetFiberWorkerCount() const noexcept;
  usize GetIOWorkerCount() const noexcept;

 private:
  static constexpr usize kDefaultIOWorkerCount{2};

  usize fiber_worker_count_{0};
  usize io_worker_count_{0};
  u32 promotion_interval_{1000};

  internal::FiberPool large_stack_fibers_{
      COMET_CONF_U16(conf::kCoreLargeFiberCount), fiber::kLargeStackSize};
  internal::FiberPool gigantic_stack_fibers_{
      COMET_CONF_U16(conf::kCoreGiganticFiberCount), fiber::kGiganticStackSize};
#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  internal::FiberPool external_library_stack_fibers_{
      COMET_CONF_U16(conf::kCoreExternalLibraryFiberCount),
      fiber::kNormalExternalLibraryStackSize};
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT

  internal::CounterPool counters_{};

  memory::PlatformAllocator worker_allocator{memory::kEngineMemoryTagFiber};
  Array<FiberWorker> fiber_workers_{};
  Array<IOWorker> io_workers_{};
  static_assert(std::atomic<bool>::is_always_lock_free,
                "std::atomic<bool> needs to be always "
                "lock-free. Unsupported "
                "architecture");
  std::atomic<bool> is_shutdown_required_{false};

  memory::PlatformAllocator job_queue_allocator_{memory::kEngineMemoryTagFiber};
  // Platform allocator is used because queues only allocated once, during
  // engine startup.
  LockFreeMPMCRingQueue<JobDescr> low_priority_queue_{};
  LockFreeMPMCRingQueue<JobDescr> normal_priority_queue_{};
  LockFreeMPMCRingQueue<JobDescr> high_priority_queue_{};
  LockFreeMPMCRingQueue<IOJobDescr> io_queue_{};

  using WorkFunc = void (Scheduler::*)();

  void Work(Worker* worker, WorkFunc work_func);
  void WorkOnFibers();
  void WorkOnIO();
  internal::FiberPool* ResolveFiberPool(const JobDescr& job_descr);
  void CleanCompletedAndTryResumeNext();
  static void OnFiberEnd(fiber::Fiber* fiber, void* data);
  void SubmitJob(const JobDescr& job_descr);
  void SubmitJob(const IOJobDescr& job_descr);
  void PromoteJobs();
};

class CounterGuard {
 public:
  CounterGuard();
  ~CounterGuard();
  CounterGuard(const CounterGuard&) = delete;
  CounterGuard& operator=(const CounterGuard&) = delete;

  void Wait();
  Counter* GetCounter();

 private:
  Counter* counter_{nullptr};
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_SCHEDULER_H_