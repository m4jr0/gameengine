// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_RENDER_MANAGER_H_
#define COMET_RENDER_RENDER_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"
#include "comet/data/light/light.h"
#include "comet/platform/window/window_common.h"
#include "comet/render/common.h"
#include "comet/render/driver/driver.h"
#include "comet/render/render_proxy_record_store.h"
#include "comet/render/view.h"
#include "comet/runtime/frame/frame_container.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/manager.h"
#include "comet/runtime/memory/memory_tag.h"

namespace comet {
namespace render {
class RenderManager : public Manager {
 public:
  static RenderManager& Get();

  RenderManager() = default;
  RenderManager(const RenderManager&) = delete;
  RenderManager(RenderManager&&) = delete;
  RenderManager& operator=(const RenderManager&) = delete;
  RenderManager& operator=(RenderManager&&) = delete;
  ~RenderManager() override = default;

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
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_RENDER_MANAGER_H_
