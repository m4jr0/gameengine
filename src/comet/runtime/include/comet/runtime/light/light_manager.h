// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_LIGHT_LIGHT_MANAGER_H_
#define COMET_RUNTIME_LIGHT_LIGHT_MANAGER_H_

#include "comet/core/container/array.h"
#include "comet/core/essentials.h"
#include "comet/core/handle/handle.h"
#include "comet/data/light/light.h"
#include "comet/runtime/frame/frame_container.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/light/light_handle.h"
#include "comet/runtime/manager.h"
#include "comet/runtime/memory/allocator/free_list_allocator.h"

namespace comet {
namespace light {
class LightManager : public Manager {
 public:
  static LightManager& Get();

  LightManager() = default;
  LightManager(const LightManager&) = delete;
  LightManager(LightManager&&) = delete;
  LightManager& operator=(const LightManager&) = delete;
  LightManager& operator=(LightManager&&) = delete;
  ~LightManager() override = default;

  void Update(frame::FramePacket* packet);

  LightHandle Generate(const LightDescr& descr);
  void Destroy(LightHandle handle);

  void SetProperties(LightHandle handle, const LightProperties& props);
  void SetShadow(LightHandle handle, const LightShadow& shadow);

  void SetColor(LightHandle handle, const math::Vec3& color);
  void SetIntensity(LightHandle handle, f32 intensity);
  void SetRange(LightHandle handle, f32 range);
  void SetDirection(LightHandle handle, const math::Vec3& direction);

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  bool IsNew(LightHandle handle) const noexcept;

  void SyncFromTransforms();
  void EmitFramePacketChanges(frame::FramePacket* packet);

  math::Vec3 ExtractPosition(const math::Mat4& transform) const;
  math::Vec3 ExtractForward(const math::Mat4& transform) const;

  Light* Get(LightHandle handle);
  const Light* Get(LightHandle handle) const;

  HandlePool<LightTag> light_pool_{};
  memory::FiberFreeListAllocator allocator_{sizeof(Light), 256,
                                            kEngineMemoryTagRender};
  Array<Light> lights_{};
  frame::DoubleFrameOrderedSet<LightHandle>* new_light_handles_{nullptr};
  frame::DoubleFrameOrderedSet<LightHandle>* destroyed_light_handles_{nullptr};
};
}  // namespace light
}  // namespace comet

#endif  // COMET_RUNTIME_LIGHT_LIGHT_MANAGER_H_