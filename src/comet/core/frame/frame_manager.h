// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_FRAME_MANAGER_H_
#define COMET_COMET_CORE_FRAME_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_allocator.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/manager.h"
#include "comet/core/type/array.h"

namespace comet {
namespace frame {
struct InFlightFrames {
  FramePacket* lead_frame{nullptr};
  FramePacket* middle_frame{nullptr};
  FramePacket* trail_frame{nullptr};
};

class FrameManager : public Manager {
 public:
  static FrameManager& Get();

  FrameManager();
  FrameManager(const FrameManager&) = delete;
  FrameManager(FrameManager&&) = delete;
  FrameManager& operator=(const FrameManager&) = delete;
  FrameManager& operator=(FrameManager&&) = delete;
  virtual ~FrameManager() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update();

  InFlightFrames& GetInFlightFrames();
  memory::AllocatorHandle GetFrameAllocatorHandle();
  memory::AllocatorHandle GetDoubleFrameAllocatorHandle();

 private:
  static inline constexpr usize kInFlightFramePacketCount_{3};
  static inline constexpr usize kFramePacketCount_{16};
  static_assert(kFramePacketCount_ >= kInFlightFramePacketCount_,
                "Frame packet count must be >= kInFlightFramePacketCount_!");

  FrameCount frame_count_{kInFlightFramePacketCount_ - 1};
  usize frame_allocator_cursor_{0};
  InFlightFrames in_flight_frames_{nullptr, nullptr, nullptr};
  FramePacket frame_packets_[kFramePacketCount_]{};

  // Allocators.
  usize fiber_frame_allocator_capacity_{0};
  usize io_frame_allocator_capacity_{0};

  FiberFrameAllocator fiber_frame_allocators_[kInFlightFramePacketCount_];
  IOFrameAllocator io_frame_allocators_[kInFlightFramePacketCount_];
  FiberDoubleFrameAllocator
      fiber_double_frame_allocators_[kInFlightFramePacketCount_];
  IODoubleFrameAllocator
      io_double_frame_allocators_[kInFlightFramePacketCount_];

  memory::AllocatorBox fiber_frame_allocator_box_{};
  memory::AllocatorBox io_frame_allocator_box_{};

  memory::AllocatorBox fiber_double_frame_allocator_box_{};
  memory::AllocatorBox io_double_frame_allocator_box_{};
};
}  // namespace frame
}  // namespace comet

#endif  // COMET_COMET_CORE_FRAME_MANAGER_H_
