// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDERING_MANAGER_H_
#define COMET_COMET_RENDERING_RENDERING_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/manager.h"
#include "comet/entity/entity_manager.h"
#include "comet/event/event_manager.h"
#include "comet/input/input_manager.h"
#include "comet/rendering/camera/camera_manager.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource_manager.h"
#include "comet/time/time_manager.h"

#ifdef COMET_DEBUG
#include "comet/rendering/debugger/debugger_displayer_manager.h"
#endif  // COMET_DEBUG

namespace comet {
namespace rendering {
struct RenderingManagerDescr : ManagerDescr {
  CameraManager* camera_manager{nullptr};
  conf::ConfigurationManager* configuration_manager{nullptr};
#ifdef COMET_DEBUG
  rendering::DebuggerDisplayerManager* debugger_displayer_manager{nullptr};
#endif  // COMET_DEBUG
  entity::EntityManager* entity_manager{nullptr};
  event::EventManager* event_manager{nullptr};
  input::InputManager* input_manager{nullptr};
  resource::ResourceManager* resource_manager{nullptr};
  time::TimeManager* time_manager{nullptr};
};

class RenderingManager : public Manager {
 public:
  RenderingManager() = delete;
  explicit RenderingManager(const RenderingManagerDescr& descr);
  RenderingManager(const RenderingManager&) = delete;
  RenderingManager(RenderingManager&&) = delete;
  RenderingManager& operator=(const RenderingManager&) = delete;
  RenderingManager& operator=(RenderingManager&&) = delete;
  virtual ~RenderingManager() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update(time::Interpolation interpolation);

  const Window* GetWindow() const;
  rendering::DriverType GetDriverType() const noexcept;
  FrameCount GetFrameRate() const noexcept;
  f32 GetFrameTime() const noexcept;
  uindex GetDrawCount() const;

 private:
  void GenerateOpenGlDriver();
  void GenerateVulkanDriver();
  void GenerateDirect3D12Driver();
  void FillDriverDescr(DriverDescr& descr) const;
  std::vector<RenderingViewDescr> GenerateRenderingViewDescrs() const;
  bool IsFpsCapReached() const;

  FrameCount frame_rate_{0};
  FrameCount counter_{0};
  f64 frame_time_threshold_{0};
  f64 current_time_{0};
  std::unique_ptr<Driver> driver_{nullptr};
  CameraManager* camera_manager_{nullptr};
  conf::ConfigurationManager* configuration_manager_{nullptr};
#ifdef COMET_DEBUG
  rendering::DebuggerDisplayerManager* debugger_displayer_manager_{nullptr};
#endif  // COMET_DEBUG
  entity::EntityManager* entity_manager_{nullptr};
  event::EventManager* event_manager_{nullptr};
  input::InputManager* input_manager_{nullptr};
  resource::ResourceManager* resource_manager_{nullptr};
  time::TimeManager* time_manager_{nullptr};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_RENDERING_MANAGER_H_
