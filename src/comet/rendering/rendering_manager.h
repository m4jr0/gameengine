// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDERING_MANAGER_H_
#define COMET_COMET_RENDERING_RENDERING_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/manager.h"
#include "comet/core/memory/memory.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
class RenderingManager : public Manager {
 public:
  static RenderingManager& Get();

  RenderingManager() = default;
  RenderingManager(const RenderingManager&) = delete;
  RenderingManager(RenderingManager&&) = delete;
  RenderingManager& operator=(const RenderingManager&) = delete;
  RenderingManager& operator=(RenderingManager&&) = delete;
  virtual ~RenderingManager() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update(frame::FramePacket* packet);

  const Window* GetWindow() const;
  DriverType GetDriverType() const noexcept;
  FrameCount GetFrameRate() const noexcept;
  f64 GetFrameTime() const noexcept;
  u32 GetDrawCount() const noexcept;
  bool IsMultithreaded() const noexcept;

 private:
  void GenerateOpenGlDriver();
  void GenerateVulkanDriver();
  void GenerateDirect3D12Driver();
#ifdef COMET_DEBUG
  void GenerateEmptyDriver();
#endif  // COMET_DEBUG
  void FillDriverDescr(DriverDescr& descr) const;
  frame::FrameArray<RenderingViewDescr> GenerateRenderingViewDescrs() const;
  bool IsFpsCapReached() const;

  bool is_multithreaded_{false};
  FrameCount frame_rate_{0};
  FrameCount counter_{0};
  f64 frame_time_threshold_{0};
  f64 current_time_{0};
  memory::UniquePtr<Driver> driver_{nullptr};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_RENDERING_MANAGER_H_
