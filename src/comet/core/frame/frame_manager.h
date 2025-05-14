// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_FRAME_MANAGER_H_
#define COMET_COMET_CORE_FRAME_MANAGER_H_

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/essentials.h"
#include "comet/core/frame/frame_allocator.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/manager.h"

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

  void WaitForNextFrame();

  InFlightFrames& GetInFlightFrames();
  memory::Allocator* GetFrameAllocator();
  memory::Allocator* GetDoubleFrameAllocator();

 private:
  void ClearAndSwapAllocators();
  void ClearAndSwapAllocator(FiberFrameAllocator& frame_allocator,
                             FiberDoubleFrameAllocator& double_allocator);
  void ClearAndSwapAllocator(IOFrameAllocator& frame_allocator,
                             IODoubleFrameAllocator& double_allocator);
  void UpdateInFlightFrames();

  static inline constexpr usize kInFlightFramePacketCount_{3};
  static inline constexpr usize kFramePacketCount_{16};
  static_assert(kFramePacketCount_ >= kInFlightFramePacketCount_,
                "Frame packet count must be >= kInFlightFramePacketCount_!");

  FrameCount frame_count_{kInFlightFramePacketCount_ - 1};
  usize frame_allocator_cursor_{0};
  InFlightFrames in_flight_frames_{nullptr, nullptr, nullptr};
  FramePacket* frame_packets_{nullptr};

  // Allocators.
  usize fiber_frame_allocator_capacity_{0};
  usize io_frame_allocator_capacity_{0};

  FiberFrameAllocator fiber_frame_allocator_;
  IOFrameAllocator io_frame_allocator_;
  FiberDoubleFrameAllocator fiber_double_frame_allocator_;
  IODoubleFrameAllocator io_double_frame_allocator_;

  fiber::FiberMutex frame_mutex_{};
  fiber::FiberCV frame_cv_{};
};
}  // namespace frame
}  // namespace comet

#endif  // COMET_COMET_CORE_FRAME_MANAGER_H_
