// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "scheduler.h"

#include "comet/core/file_system/file_system.h"  // >:3
#include "comet/core/job/fiber/fiber.h"
#include "comet/core/job/fiber/fiber_context.h"
#include "comet/core/job/fiber/fiber_queue.h"
#include "comet/core/job/job.h"
#include "comet/time/time_manager.h"

namespace comet {
namespace job {
static u32 ja{123456};
static u32 jaa{0};
static u32 jb{789465};
static TString path;
static auto* file{COMET_TCHAR("tmp.txt")};

void WriteTmpFile(ParamsHandle data) {
  std::ofstream file;
  auto* path{reinterpret_cast<TString*>(data)};
  OpenFileToWriteTo(*path, file);
  schar buff[512];
  ConvertToStr(jaa, buff, 511);
  WriteStrToFile(*path, buff);
  CloseFile(file);
}

void DoSomeComputations(ParamsHandle data) {
  jaa = 42424242;
  IOJobDescr io_descr{};
  io_descr.entry_point = WriteTmpFile;
  io_descr.params_handle = reinterpret_cast<ParamsHandle>(&path);
  Scheduler::Get().Kick(io_descr);
}

void CreateTmpFile(ParamsHandle data) {
  path = GetCurrentDirectory();
  COMET_ALLOW_STR_ALLOC(path);
  path /= reinterpret_cast<const tchar*>(data);
  CreateFile(path, true);

  JobDescr job_descr{};
  job_descr.entry_point = DoSomeComputations;
  job_descr.params_handle = kInvalidParamsHandle;
  job_descr.priority = JobPriority::Normal;
  Scheduler::Get().Kick(job_descr);
}

void ComputeA(ParamsHandle data) {
  auto* test{reinterpret_cast<u32*>(data)};
  *test = 42;

  s32 a{0};
  std::cout << "ComputeA - A0: " << a << '\n';

  Yield();

  IOJobDescr io_descr{};
  io_descr.entry_point = CreateTmpFile;
  io_descr.params_handle = reinterpret_cast<ParamsHandle>(&file[0]);
  Scheduler::Get().Kick(io_descr);

  ++a;
  std::cout << "ComputeA - A2: " << a << '\n';
}

void ComputeB(ParamsHandle data) {
  auto* test{reinterpret_cast<u32*>(data)};
  s32 a{1337};
  std::cout << "ComputeB - A1337: " << a << '\n';
  ++a;
  std::cout << "ComputeB - A1338: " << a << '\n';
}

// Fiber* main;
// Fiber* fiber_a;
// Fiber* fiber_b;

void TestFibers() {
  // main = SwitchThreadToFiber();

  // SwitchData data_a{&fiber_a, &main};
  // SwitchData data_b{&fiber_b, &main};

  //[[maybe_unused]] auto* data_a_p{&data_a};
  //[[maybe_unused]] auto* data_b_p{&data_b};

  JobDescr descr_a{};
  descr_a.entry_point = ComputeA;
  descr_a.params_handle = reinterpret_cast<ParamsHandle>(&ja);
  descr_a.priority = JobPriority::High;
  descr_a.counter = nullptr;

  JobDescr descr_b{};
  descr_b.entry_point = ComputeB;
  descr_b.params_handle = reinterpret_cast<ParamsHandle>(&jb);
  descr_b.priority = JobPriority::Normal;
  descr_b.counter = nullptr;

  Scheduler::Get().Kick(descr_a);
  Scheduler::Get().Kick(descr_b);

  // u32 params{1337};

  // fiber_a = Generate(kNormalStack);
  // fiber_b = Generate(kNormalStack);

  // Attach(fiber_a, ComputeA, reinterpret_cast<ParamsHandle>(&params));
  // Attach(fiber_b, ComputeB, reinterpret_cast<ParamsHandle>(&params));

  // SwitchTo(fiber_a);
  // SwitchTo(fiber_b);

  // s32 a{42};
  // std::cout << "TestFibers - A42: " << a << '\n';
  //++a;
  // std::cout << "TestFibers - A43: " << a << '\n';

  // SwitchTo(fiber_a);

  //++a;
  // std::cout << "TestFibers - A44: " << a << '\n';

  // Destroy(fiber_a);
  // Destroy(fiber_b);
  //  Destroy(main);
}

Scheduler& Scheduler::Get() {
  static Scheduler singleton{};
  return singleton;
}

void Scheduler::Initialize() {
  large_stack_fibers_.reserve(kLargeStackFiberCount_);

  for (uindex i{0}; i < kLargeStackFiberCount_; ++i) {
    auto* fiber{AllocateAligned<Fiber>(MemoryTag::Fiber)};
    new (fiber) Fiber{kLargeStack};
    fiber->Initialize();
    large_stack_fibers_.emplace_back(fiber);
  }

  gigantic_stack_fibers_.reserve(kGiganticStackFiberCount_);

  for (uindex i{0}; i < kGiganticStackFiberCount_; ++i) {
    auto* fiber{AllocateAligned<Fiber>(MemoryTag::Fiber)};
    new (fiber) Fiber{kGiganticStack};
    gigantic_stack_fibers_.emplace_back(fiber);
  }

  is_shutdown_required_ = false;
  uindex worker_count{std::thread::hardware_concurrency()};
  worker_count = 2;  // >:3
  workers_.reserve(worker_count);
  workers_.emplace_back(std::thread{});
  workers_[0].Initialize();

  for (uindex i{0}; i < worker_count - 1; ++i) {
    workers_.emplace_back(std::thread{&Scheduler::WorkerThread, this, i + 1});
  }

  constexpr auto io_worker_count{1};  // >:3
  worker_count += io_worker_count;
  io_workers_.reserve(worker_count);

  for (uindex i{0}; i < io_worker_count; ++i) {
    io_workers_.emplace_back(std::thread{&Scheduler::IOWorkerThread, this});
  }

  while (worker_ready_count_ !=
         worker_count - 1) {  // >:3 -1 because of main thread.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  TestFibers();
}

void Scheduler::Shutdown() {
  is_shutdown_required_ = true;

  for (auto& worker : workers_) {
    worker.Destroy();
  }

  // >:3 Lock.

  for (uindex i{0}; i < large_stack_fibers_.size(); ++i) {
    auto* fiber{large_stack_fibers_[i]};

    if (fiber == nullptr) {
      continue;
    }

    fiber->Destroy();
  }

  large_stack_fibers_.clear();

  for (uindex i{0}; i < gigantic_stack_fibers_.size(); ++i) {
    auto* fiber{gigantic_stack_fibers_[i]};

    if (fiber == nullptr) {
      continue;
    }

    fiber->Destroy();
  }

  gigantic_stack_fibers_.clear();
}

Counter* Scheduler::AllocateCounter() {
  SimpleLockGuard lock{counter_lock_};
  COMET_ASSERT(available_counters_.size() > 0,
               "No counter is available anymore!");
  auto* counter{available_counters_.front()};
  available_counters_.pop();
  return counter;
}

void Scheduler::FreeCounter(Counter* counter) {
  COMET_ASSERT(counter != nullptr, "Counter provided is null!");
  SimpleLockGuard lock{counter_lock_};
  counter->value = 0;
  available_counters_.push(counter);
}

void Scheduler::Kick(const JobDescr& job_descr) { SubmitJob(job_descr); }

void Scheduler::Kick(uindex job_count, const JobDescr* job_descrs) {
  for (uindex i{0}; i < job_count; ++i) {
    Kick(job_descrs[i]);
  }
}

void Scheduler::Kick(const IOJobDescr& job_descr) { SubmitJob(job_descr); }

void Scheduler::Kick(uindex job_count, const IOJobDescr* job_descrs) {
  for (uindex i{0}; i < job_count; ++i) {
    Kick(job_descrs[i]);
  }
}

void Scheduler::Wait([[maybe_unused]] Counter* counter) {}

void Scheduler::KickAndWait([[maybe_unused]] const JobDescr& job_descr) {}

void Scheduler::KickAndWait([[maybe_unused]] uindex job_count,
                            [[maybe_unused]] const JobDescr* job_descrs) {}

void Scheduler::WorkerThread(uindex worker_index) {
  auto& worker{workers_[worker_index]};
  worker.Initialize();
  ++worker_ready_count_;

  while (!is_shutdown_required_) {
    auto current_time{time::TimeManager::Get().GetCurrentTime()};

    if (current_time - last_promotion_time_ >= promotion_interval_) {
      PromoteJobs();
      last_promotion_time_ = current_time;
    }

    std::queue<JobDescr>* queue{nullptr};

    if (!high_priority_queue_.empty()) {
      queue = &high_priority_queue_;
    } else if (!normal_priority_queue_.empty()) {
      queue = &normal_priority_queue_;
    } else if (!low_priority_queue_.empty()) {
      queue = &low_priority_queue_;
    }

    if (queue == nullptr) {
      Yield();
      continue;
    }

    {
      if (IsBlockableThread()) {
        queue_lock_.Lock();
      } else {
        while (!queue_lock_.TryLock()) {
          Yield();
        }
      }

      auto& job{queue->front()};
      auto* fiber{ResolveFiber(job)};
      COMET_ASSERT(fiber != nullptr, "No fiber available!");
      fiber->Attach(job.entry_point, job.params_handle, OnFiberEnd);
      queue->pop();
      auto& fiber_queue{FiberQueue::Get()};
      fiber_queue.Enqueue(fiber);

      queue_lock_.Unlock();
    }

    Yield();
  }

  worker.Destroy();
}

void Scheduler::IOWorkerThread() {
  ++worker_ready_count_;

  while (!is_shutdown_required_) {
    IOJobDescr job;

    {
      if (is_shutdown_required_) {
        break;
      }

      SimpleLockGuard lock{io_queue_lock_};

      if (io_queue_.empty()) {
        continue;
      }

      job = io_queue_.front();
      io_queue_.pop();
    }

    job.entry_point(job.params_handle);
  }
}

Fiber* Scheduler::ResolveFiber(const JobDescr& job_descr) {
  FiberLockGuard lock{fiber_pool_mutex_};
  Fiber* fiber = large_stack_fibers_.back();
  large_stack_fibers_.pop_back();
  fiber->Reset();
  return fiber;
}

void Scheduler::OnFiberEnd(Fiber* fiber) {
  auto& scheduler{Get()};

  {
    FiberLockGuard lock{scheduler.fiber_pool_mutex_};
    fiber->Detach();
    scheduler.large_stack_fibers_.emplace_back(fiber);
  }

  SwitchTo(FiberQueue::Get().Dequeue());
}

void Scheduler::SubmitJob(const JobDescr& job_descr) {
  {
    if (IsBlockableThread()) {
      queue_lock_.Lock();
    } else {
      while (!queue_lock_.TryLock()) {
        Yield();
      }
    }

    switch (job_descr.priority) {
      case JobPriority::High:
        high_priority_queue_.push(job_descr);
        break;
      case JobPriority::Normal:
        normal_priority_queue_.push(job_descr);
        break;
      case JobPriority::Low:
        low_priority_queue_.push(job_descr);
        break;
      default:
        COMET_ASSERT(false, "Unknown or unsupported job priority: ",
                     static_cast<std::underlying_type_t<JobPriority>>(
                         job_descr.priority),
                     "!");
        break;
    }

    queue_lock_.Unlock();
  }
}

void Scheduler::SubmitJob(const IOJobDescr& job_descr) {
  if (IsBlockableThread()) {
    io_queue_lock_.Lock();
  } else {
    while (!io_queue_lock_.TryLock()) {
      Yield();
    }
  }

  io_queue_.emplace(job_descr);
  io_cv_.notify_one();
  io_queue_lock_.Unlock();
}

void Scheduler::PromoteJobs() {
  if (IsBlockableThread()) {
    queue_lock_.Lock();
  } else {
    while (!queue_lock_.TryLock()) {
      Yield();
    }
  }

  while (!normal_priority_queue_.empty()) {
    high_priority_queue_.push(normal_priority_queue_.front());
    normal_priority_queue_.pop();
  }

  while (!low_priority_queue_.empty()) {
    normal_priority_queue_.push(low_priority_queue_.front());
    low_priority_queue_.pop();
  }

  queue_lock_.Unlock();
}
}  // namespace job
}  // namespace comet