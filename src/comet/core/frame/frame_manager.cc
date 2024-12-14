// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "frame_manager.h"

#include "comet/core/concurrency/job/worker_context.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"
#include "comet/core/game_state_manager.h"
#include "comet/profiler/profiler_manager.h"

namespace comet {
namespace frame {
FrameManager& FrameManager::Get() {
  static FrameManager singleton{};
  return singleton;
}

FrameManager::FrameManager()
    : fiber_frame_allocator_capacity_{COMET_CONF_U32(
          conf::kCoreFiberFrameAllocatorBaseCapacity)},
      io_frame_allocator_capacity_{
          COMET_CONF_U32(conf::kCoreIOFrameAllocatorBaseCapacity)},
      fiber_frame_allocator_{fiber_frame_allocator_capacity_,
                             memory::kEngineMemoryTagFrame0},
      io_frame_allocator_{io_frame_allocator_capacity_,
                          memory::kEngineMemoryTagFrame0},
      fiber_double_frame_allocator_{fiber_frame_allocator_capacity_,
                                    memory::kEngineMemoryTagDoubleFrame0},
      io_double_frame_allocator_{io_frame_allocator_capacity_,
                                 memory::kEngineMemoryTagDoubleFrame0} {}

void FrameManager::Initialize() {
  Manager::Initialize();

  fiber_frame_allocator_.Initialize();
  fiber_double_frame_allocator_.Initialize();
  io_frame_allocator_.Initialize();
  io_double_frame_allocator_.Initialize();

  UpdateInFlightFrames();
}

void FrameManager::Shutdown() {
  Manager::Shutdown();

  fiber_frame_allocator_.Destroy();
  fiber_double_frame_allocator_.Destroy();
  io_frame_allocator_.Destroy();
  io_double_frame_allocator_.Destroy();
}

void FrameManager::Update() {
  if (GameStateManager::Get().IsPaused()) {
    ClearAndSwapAllocators();
    return;
  }

  COMET_PROFILER_END_FRAME();
  UpdateInFlightFrames();

  {
    fiber::FiberLockGuard lock{frame_mutex_};
    ++frame_count_;
  }

  COMET_PROFILER_START_FRAME(frame_count_);
  ClearAndSwapAllocators();
  frame_cv_.NotifyAll();
}

void FrameManager::WaitForNextFrame() {
  fiber::FiberUniqueLock lock{frame_mutex_};
  auto current_frame_count{frame_count_};

  frame_cv_.Wait(lock, [this, current_frame_count] {
    return current_frame_count < frame_count_;
  });
}

InFlightFrames& FrameManager::GetInFlightFrames() { return in_flight_frames_; }

memory::Allocator* FrameManager::GetFrameAllocator() {
  if (job::IsFiberWorker()) {
    return &fiber_frame_allocator_;
  }

  if (job::IsIOWorker()) {
    return &io_frame_allocator_;
  }

  COMET_ASSERT(false,
               "No frame allocator available: no worker has been "
               "attached on this thread!");
  return nullptr;
}

memory::Allocator* FrameManager::GetDoubleFrameAllocator() {
  if (job::IsFiberWorker()) {
    return &fiber_double_frame_allocator_;
  }

  if (job::IsIOWorker()) {
    return &io_double_frame_allocator_;
  }

  COMET_ASSERT(false,
               "No double frame allocator available: no worker has been "
               "attached on this thread!");
  return nullptr;
}

void FrameManager::ClearAndSwapAllocators() {
  ClearAndSwapAllocator(fiber_frame_allocator_, fiber_double_frame_allocator_);
  ClearAndSwapAllocator(io_frame_allocator_, io_double_frame_allocator_);
}

void FrameManager::ClearAndSwapAllocator(
    FiberFrameAllocator& frame_allocator,
    FiberDoubleFrameAllocator& double_allocator) {
  frame_allocator.Clear();
  double_allocator.SwapStacks();
  double_allocator.ClearCurrent();
}

void FrameManager::ClearAndSwapAllocator(
    IOFrameAllocator& frame_allocator,
    IODoubleFrameAllocator& double_allocator) {
  frame_allocator.Clear();
  double_allocator.SwapStacks();
  double_allocator.ClearCurrent();
}

void FrameManager::UpdateInFlightFrames() {
  in_flight_frames_.lead_frame =
      &frame_packets_[frame_count_ % kFramePacketCount_];
  in_flight_frames_.middle_frame =
      &frame_packets_[(frame_count_ - 1) % kFramePacketCount_];
  in_flight_frames_.trail_frame =
      &frame_packets_[(frame_count_ - 2) % kFramePacketCount_];

  in_flight_frames_.lead_frame->frame_count = frame_count_;
  in_flight_frames_.middle_frame->frame_count = frame_count_ - 1;
  in_flight_frames_.trail_frame->frame_count = frame_count_ - 2;
}
}  // namespace frame
}  // namespace comet
