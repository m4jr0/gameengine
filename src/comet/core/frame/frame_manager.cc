// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "frame_manager.h"

#include "comet/core/concurrency/job/worker_context.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"

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
      fiber_frame_allocators_{
          FiberFrameAllocator{fiber_frame_allocator_capacity_,
                              memory::kEngineMemoryTagFrame0},
          FiberFrameAllocator{fiber_frame_allocator_capacity_,
                              memory::kEngineMemoryTagFrame1},
          FiberFrameAllocator{fiber_frame_allocator_capacity_,
                              memory::kEngineMemoryTagFrame2}},
      io_frame_allocators_{IOFrameAllocator{io_frame_allocator_capacity_,
                                            memory::kEngineMemoryTagFrame0},
                           IOFrameAllocator{io_frame_allocator_capacity_,
                                            memory::kEngineMemoryTagFrame1},
                           IOFrameAllocator{io_frame_allocator_capacity_,
                                            memory::kEngineMemoryTagFrame2}},
      fiber_double_frame_allocators_{
          FiberDoubleFrameAllocator{fiber_frame_allocator_capacity_,
                                    memory::kEngineMemoryTagDoubleFrame0},
          FiberDoubleFrameAllocator{fiber_frame_allocator_capacity_,
                                    memory::kEngineMemoryTagDoubleFrame1},
          FiberDoubleFrameAllocator{fiber_frame_allocator_capacity_,
                                    memory::kEngineMemoryTagDoubleFrame2}},
      io_double_frame_allocators_{
          IODoubleFrameAllocator{io_frame_allocator_capacity_,
                                 memory::kEngineMemoryTagDoubleFrame0},
          IODoubleFrameAllocator{io_frame_allocator_capacity_,
                                 memory::kEngineMemoryTagDoubleFrame1},
          IODoubleFrameAllocator{io_frame_allocator_capacity_,
                                 memory::kEngineMemoryTagDoubleFrame2}},
      fiber_frame_allocator_box_{&fiber_frame_allocators_[0]},
      io_frame_allocator_box_{&io_frame_allocators_[0]},
      fiber_double_frame_allocator_box_{&fiber_double_frame_allocators_[0]},
      io_double_frame_allocator_box_{&io_double_frame_allocators_[0]} {}

void FrameManager::Initialize() {
  Manager::Initialize();

  for (usize i{0}; i < kInFlightFramePacketCount_; ++i) {
    fiber_frame_allocators_[i].Initialize();
    fiber_double_frame_allocators_[i].Initialize();
    io_frame_allocators_[i].Initialize();
    io_double_frame_allocators_[i].Initialize();
  }
}

void FrameManager::Shutdown() {
  Manager::Shutdown();

  for (usize i{0}; i < kInFlightFramePacketCount_; ++i) {
    fiber_frame_allocators_[i].Destroy();
    fiber_double_frame_allocators_[i].Destroy();
    io_frame_allocators_[i].Destroy();
    io_double_frame_allocators_[i].Destroy();
  }
}

void FrameManager::Update() {
  in_flight_frames_.lead_frame =
      &frame_packets_[frame_count_ % kFramePacketCount_];
  in_flight_frames_.middle_frame =
      &frame_packets_[(frame_count_ - 1) % kFramePacketCount_];
  in_flight_frames_.trail_frame =
      &frame_packets_[(frame_count_ - 2) % kFramePacketCount_];

  in_flight_frames_.lead_frame->frame_count = frame_count_;
  in_flight_frames_.middle_frame->frame_count = frame_count_ - 1;
  in_flight_frames_.trail_frame->frame_count = frame_count_ - 2;

  ++frame_count_;
  frame_allocator_cursor_ = frame_count_ % kInFlightFramePacketCount_;

  auto& new_fiber_frame_allocator{
      fiber_frame_allocators_[frame_allocator_cursor_]};
  auto& new_fiber_double_frame_allocator{
      fiber_double_frame_allocators_[frame_allocator_cursor_]};

  new_fiber_frame_allocator.Clear();
  new_fiber_double_frame_allocator.SwapStacks();
  new_fiber_double_frame_allocator.ClearCurrent();

  fiber_frame_allocator_box_.allocator = &new_fiber_frame_allocator;
  fiber_double_frame_allocator_box_.allocator =
      &new_fiber_double_frame_allocator;

  auto& new_io_frame_allocator{io_frame_allocators_[frame_allocator_cursor_]};
  auto& new_io_double_frame_allocator{
      io_double_frame_allocators_[frame_allocator_cursor_]};

  new_io_frame_allocator.Clear();
  new_io_double_frame_allocator.SwapStacks();
  new_io_double_frame_allocator.ClearCurrent();

  io_frame_allocator_box_.allocator = &new_io_frame_allocator;
  io_double_frame_allocator_box_.allocator = &new_io_double_frame_allocator;
}

InFlightFrames& FrameManager::GetInFlightFrames() { return in_flight_frames_; }

memory::AllocatorHandle FrameManager::GetFrameAllocatorHandle() {
  if (job::IsFiberWorker()) {
    return &fiber_frame_allocator_box_;
  }

  if (job::IsIOWorker()) {
    return &io_frame_allocator_box_;
  }

  COMET_ASSERT(false,
               "No frame allocator handle available: no worker has been "
               "attached on this thread!");
  return nullptr;
}

memory::AllocatorHandle FrameManager::GetDoubleFrameAllocatorHandle() {
  if (job::IsFiberWorker()) {
    return &fiber_double_frame_allocator_box_;
  }

  if (job::IsIOWorker()) {
    return &io_double_frame_allocator_box_;
  }

  COMET_ASSERT(false,
               "No double frame allocator handle available: no worker has been "
               "attached on this thread!");
  return nullptr;
}
}  // namespace frame
}  // namespace comet
