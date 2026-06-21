// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_CAMERA_CAMERA_MANAGER_H_
#define COMET_RUNTIME_CAMERA_CAMERA_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/platform/window/window_common.h"
#include "comet/runtime/manager.h"
#include "comet/runtime/memory/allocator/free_list_allocator.h"
#include "comet/core/container/array.h"
#include "comet/core/handle/handle.h"
#include "comet/runtime/event/event.h"
#include "comet/runtime/camera/camera_handle.h"
#include "comet/runtime/event/event_manager.h"
#include "comet/core/math/matrix.h"
#include "comet/core/math/quaternion.h"
#include "comet/core/math/vector.h"
#include "comet/runtime/camera/camera.h"
#include "comet/render/common.h"

namespace comet {
namespace camera {
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
#ifdef COMET_DEBUG
  CameraHandle GetDebugCamera() const noexcept;
#endif  // COMET_DEBUG

  bool IsAlive(CameraHandle handle) const noexcept;

  CameraHandle Generate(const CameraDescr& descr = CameraDescr{});
  void Destroy(CameraHandle handle);

  void SetProjectionFlags(CameraProjectionFlags flags) noexcept;
  CameraProjectionFlags GetProjectionFlags() const noexcept;

  usize PopulateCameraViews(Array<CameraView>& views, WindowSize full_width,
                            WindowSize full_height);

  void Reset(CameraHandle handle);
  void Translate(CameraHandle handle, const math::Vec3& translation);
  void Move(CameraHandle handle, const math::Vec3& delta);
  void Rotate(CameraHandle handle, const math::Vec2& delta);
  void Rotate(CameraHandle handle, const math::Quat& rotation);
  void Orbit(CameraHandle handle, const math::Vec2& delta);

  void SetPosition(CameraHandle handle, const math::Vec3& position);
  void SetRotation(CameraHandle handle, const math::Quat& rotation);
  void SetSize(CameraHandle handle, WindowSize width, WindowSize height);
  void SetSize(CameraHandle handle, WindowExtent extent);
  void SetWidth(CameraHandle handle, WindowSize width);
  void SetHeight(CameraHandle handle, WindowSize height);
  void SetFov(CameraHandle handle, f32 fov);
  void SetNearPlane(CameraHandle handle, f32 value);
  void SetFarPlane(CameraHandle handle, f32 value);
  void SetResetPose(CameraHandle handle, const CameraPose& pose);

  const math::Vec3& GetPosition(CameraHandle handle) const;
  const math::Quat& GetRotation(CameraHandle handle) const;
  const math::Vec3& GetUp(CameraHandle handle) const;
  const math::Vec3& GetFront(CameraHandle handle) const;
  const math::Vec3& GetRight(CameraHandle handle) const;
  WindowExtent GetSize(CameraHandle handle) const;
  WindowSize GetWidth(CameraHandle handle) const;
  WindowSize GetHeight(CameraHandle handle) const;
  f32 GetFov(CameraHandle handle) const;
  f32 GetFovInRadians(CameraHandle handle) const;
  f32 GetNearPlane(CameraHandle handle) const;
  f32 GetFarPlane(CameraHandle handle) const;
  CameraPose GetResetPose(CameraHandle handle) const;
  f32 GetAspectRatio(CameraHandle handle) const;

#ifdef COMET_DEBUG
  void SetDebugLabel(CameraHandle handle, const schar* label);
  const schar* GetDebugLabel(CameraHandle handle) const;
#endif  // COMET_DEBUG

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  struct CameraEntry {
    CameraHandle handle{};
    WindowSize width{0};
    WindowSize height{0};

    f32 z_near{.1f};
    f32 z_far{1000.0f};
    f32 fov{45.0f};

    math::Vec3 position{};
    math::Quat rotation{};
    math::Vec3 up{};
    math::Vec3 front{};
    math::Vec3 right{};

    CameraPose reset_pose{};

#ifdef COMET_DEBUG
    schar debug_label[kCameraDebugLabelCapacity]{};
#endif  // COMET_DEBUG
  };

  void OnEvent(const event::Event& event);
  void RegisterEvents();
  void UnregisterEvents();

  CameraHandle GenerateMainCamera();

#ifdef COMET_DEBUG
  CameraHandle GenerateDebugCamera();
#endif  // COMET_DEBUG

  void PopulateRenderData(CameraHandle handle, CameraViewData& data,
                          f32 aspect_ratio) const;

  math::Mat4 GenerateProjectionMatrix(const CameraEntry& camera,
                                      f32 aspect_ratio) const;
  math::Mat4 GenerateViewMatrix(const CameraEntry& camera) const;
  void RefreshBasis(CameraEntry& camera);

  math::Vec3 GetCenterPivotPoint(const CameraEntry& camera) const;
  math::Quat GetRotationDelta(const CameraEntry& camera,
                              const math::Vec2& delta) const;

  CameraEntry* GetCamera(CameraHandle handle);
  const CameraEntry* GetCamera(CameraHandle handle) const;

  static constexpr math::Vec3 kWorldUp_{.0f, 1.0f, .0f};
  static constexpr math::Vec3 kWorldRight_{1.0f, .0f, .0f};
  static constexpr math::Vec3 kWorldFront_{.0f, .0f, 1.0f};

  event::EventListenerId window_initialized_listener_id_{
      event::kInvalidEventListenerId};
  event::EventListenerId window_resize_listener_id_{
      event::kInvalidEventListenerId};

  CameraProjectionFlags projection_flags_{kProjectionFlagBitsDepthZeroToOne};

  memory::FiberFreeListAllocator allocator_{sizeof(CameraEntry), 16,
                                            kEngineMemoryTagRender};

  HandlePool<CameraTag> camera_pool_{};
  Array<CameraEntry> cameras_{};

  CameraHandle main_camera_{};

#ifdef COMET_DEBUG
  CameraHandle debug_camera_{};
#endif  // COMET_DEBUG
};
}  // namespace camera
}  // namespace comet

#endif  // COMET_RUNTIME_CAMERA_CAMERA_MANAGER_H_