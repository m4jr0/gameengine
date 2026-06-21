// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/frame/frame_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/job/worker_context.h"
#include "comet/runtime/conf/conf_manager.h"
#include "comet/runtime/conf/config_defaults.h"
#include "comet/runtime/conf/config_keys.h"
#include "comet/runtime/conf/config_value.h"
#include "comet/runtime/continuation/continuation_manager.h"
#include "comet/runtime/continuation/continuation_phase.h"
#include "comet/runtime/event/event_manager.h"
#include "comet/runtime/frame/frame_event.h"
#include "comet/runtime/memory/tagged_memory.h"
#include "comet/runtime/profiler/profiler_manager.h"

namespace comet {
namespace frame {
FrameManager& FrameManager::Get() {
  static FrameManager singleton{};
  return singleton;
}

FrameManager::FrameManager()
    : fiber_frame_allocator_capacity_{
          COMET_CONF_U32(conf::kCoreFiberFrameAllocatorBaseCapacity)},
      io_frame_allocator_capacity_{
          COMET_CONF_U32(conf::kCoreIOFrameAllocatorBaseCapacity)},
      fiber_frame_allocator_{fiber_frame_allocator_capacity_,
                             kEngineMemoryTagFrame,
                             kEngineMemoryTagFrameExtended},
      io_frame_allocator_{io_frame_allocator_capacity_,
                          kEngineMemoryTagFrame},
      fiber_double_frame_allocator_{
          fiber_frame_allocator_capacity_, kEngineMemoryTagDoubleFrame,
          kEngineMemoryTagDoubleFrameExtended1,
          kEngineMemoryTagDoubleFrameExtended2},
      io_double_frame_allocator_{io_frame_allocator_capacity_,
                                 kEngineMemoryTagDoubleFrame} {}

void FrameManager::Update() {
  auto& continuation_manager{ContinuationManager::Get()};
  continuation_manager.Poll(ContinuationPhase::BeginFrame);

  auto& event_manager{event::EventManager::Get()};
  event_manager.FireEventNow<EndFrameEvent>();
  continuation_manager.Poll(ContinuationPhase::AfterEndFrame);

  StepFrame();

  event_manager.FireEventNow<NewFrameEvent>();
  continuation_manager.Poll(ContinuationPhase::BeginFrame);
}

void FrameManager::WaitForNextFrame() {
  fiber::FiberUniqueLock lock{frame_mutex_};
  const auto current_frame_count{frame_count_};

  frame_cv_.Wait(lock, [this, current_frame_count] {
    return current_frame_count < frame_count_;
  });
}

FramePacket* FrameManager::GetLogicFramePacket() {
  return in_flight_frames_.logic_frame_packet;
}

FramePacket* FrameManager::GetRenderingFramePacket() {
  return in_flight_frames_.rendering_frame_packet;
}

InFlightFramePackets& FrameManager::GetInFlightFramePackets() {
  return in_flight_frames_;
}

memory::Allocator* FrameManager::GetFrameAllocator() {
  if (job::IsFiberWorker()) {
    return &fiber_frame_allocator_;
  }

  if (job::IsIOWorker()) {
    return &io_frame_allocator_;
  }

  COMET_ASSERT(false, "FrameManager::GetFrameAllocator",
               "no worker is attached to this thread");
  return nullptr;
}

memory::Allocator* FrameManager::GetDoubleFrameAllocator() {
  if (job::IsFiberWorker()) {
    return &fiber_double_frame_allocator_;
  }

  if (job::IsIOWorker()) {
    return &io_double_frame_allocator_;
  }

  COMET_ASSERT(false, "FrameManager::GetDoubleFrameAllocator",
               "no worker is attached to this thread");
  return nullptr;
}

void FrameManager::OnInitialize() {
  fiber_frame_allocator_.Initialize();
  fiber_double_frame_allocator_.Initialize();
  io_frame_allocator_.Initialize();
  io_double_frame_allocator_.Initialize();

  frame_packets_ = memory::AllocateMany<FramePacket>(kFramePacketCount_,
                                                     kEngineMemoryTagGid);

  for (usize i{0}; i < kFramePacketCount_; ++i) {
    auto& packet{frame_packets_[i]};
    memory::Populate<FramePacket>(&packet);
    packet.Reset();
  }

  UpdateInFlightFrames();
}

void FrameManager::OnShutdown() {
  memory::Deallocate(frame_packets_);
  frame_packets_ = nullptr;

  fiber_frame_allocator_.Destroy();
  fiber_double_frame_allocator_.Destroy();
  io_frame_allocator_.Destroy();
  io_double_frame_allocator_.Destroy();
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
  in_flight_frames_.logic_frame_packet =
      &frame_packets_[frame_count_ % kFramePacketCount_];
  in_flight_frames_.rendering_frame_packet =
      &frame_packets_[(frame_count_ - 1) % kFramePacketCount_];

  in_flight_frames_.logic_frame_packet->Reset();

  in_flight_frames_.logic_frame_packet->frame_count = frame_count_;
  in_flight_frames_.rendering_frame_packet->frame_count = frame_count_ - 1;
}

void FrameManager::StepFrame() {
  COMET_PROFILER_END_FRAME();

  {
    fiber::FiberLockGuard lock{frame_mutex_};
    ++frame_count_;
  }

  ClearAndSwapAllocators();
  UpdateInFlightFrames();
  COMET_PROFILER_START_FRAME(frame_count_);
  frame_cv_.NotifyAll();
}
}  // namespace frame
}  // namespace comet
