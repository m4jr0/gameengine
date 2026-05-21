// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_RENDERING_MANAGER_H_
#define COMET_RENDER_RENDERING_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_container.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/manager.h"
#include "comet/core/memory/memory.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/render_proxy_record_store.h"
#include "comet/rendering/type/common.h"
#include "comet/rendering/type/light.h"
#include "comet/rendering/type/view.h"

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
  ~RenderingManager() override = default;

  void Update(frame::FramePacket* packet);

  WindowExtent GetWindowExtent() const;
  DriverType GetDriverType() const noexcept;
  FrameCount GetFrameRate() const noexcept;
  f64 GetFrameTime() const noexcept;
  u32 GetDrawCount() const noexcept;

  const ShadowSettings& GetShadowSettings() const noexcept;

  bool IsMultithreaded() const noexcept;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  void GenerateOpenGlDriver();
  void GenerateVulkanDriver();
  void GenerateDirect3D12Driver();
#ifdef COMET_DEBUG
  void GenerateEmptyDriver();
#endif  // COMET_DEBUG

  void FillDriverDescr(DriverDescr& descr) const;

  inline static constexpr usize kDefaultRenderProxyCount_{512};

  frame::FrameArray<RenderingViewDescr> GenerateRenderingViewDescrs() const;
  ShadowSettings GenerateShadowSettings() const;

  bool IsFpsCapReached() const;

  bool is_multithreaded_{false};
  FrameCount frame_rate_{0};
  FrameCount counter_{0};
  f64 frame_time_threshold_{0};
  f64 current_time_{0};
  ShadowSettings shadow_settings_{};
  RenderProxyRecordStore render_proxy_record_store_{};
  memory::UniquePtr<Driver> driver_{nullptr};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_RENDERING_MANAGER_H_
