// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_CAMERA_MANAGER_H_
#define COMET_COMET_RENDERING_CAMERA_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/handle.h"
#include "comet/event/event.h"
#include "comet/event/event_manager.h"
#include "comet/math/quaternion.h"
#include "comet/math/vector.h"
#include "comet/rendering/camera.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/rendering/type/rendering_camera_type.h"
#include "comet/rendering/type/rendering_common_type.h"

namespace comet {
namespace rendering {
class CameraManager : public Manager {
 public:
  static CameraManager& Get();

  CameraManager() = default;
  CameraManager(const CameraManager&) = delete;
  CameraManager(CameraManager&&) = delete;
  CameraManager& operator=(const CameraManager&) = delete;
  CameraManager& operator=(CameraManager&&) = delete;
  ~CameraManager() override = default;

  CameraHandle GetMainCamera() const noexcept;
  bool IsAlive(CameraHandle handle) const noexcept;

  void Reset(CameraHandle handle);
  void Translate(CameraHandle handle, const math::Vec3& translation);
  void Move(CameraHandle handle, const math::Vec3& delta);
  void Rotate(CameraHandle handle, const math::Vec2& delta);
  void Rotate(CameraHandle handle, const math::Quat& rotation);
  void Orbit(CameraHandle handle, const math::Vec2& delta);

  void SetPosition(CameraHandle handle, const math::Vec3& position);
  void SetRotation(CameraHandle handle, const math::Quat& rotation);
  void SetWidth(CameraHandle handle, WindowSize width);
  void SetHeight(CameraHandle handle, WindowSize height);
  void SetSize(CameraHandle handle, WindowSize width, WindowSize height);

  WindowSize GetWidth(CameraHandle handle) const;
  WindowSize GetHeight(CameraHandle handle) const;

  void PopulateRenderData(CameraHandle handle, RenderCameraData& data);
  void PopulateMainRenderData(RenderCameraData& data);

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  void OnEvent(const event::Event& event);

  void RegisterEvents();
  void UnregisterEvents();

  CameraHandle GenerateMainCamera();

  Camera* GetCamera(CameraHandle handle);
  const Camera* GetCamera(CameraHandle handle) const;

  event::EventListenerId window_initialized_listener_id_{
      event::kInvalidEventListenerId};
  event::EventListenerId window_resize_listener_id_{
      event::kInvalidEventListenerId};

  memory::FiberFreeListAllocator allocator_{sizeof(Camera), 16,
                                            memory::kEngineMemoryTagRendering};

  HandlePool<CameraTag> camera_pool_{};
  Array<Camera> cameras_{};

  CameraHandle main_camera_{};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_CAMERA_MANAGER_H_