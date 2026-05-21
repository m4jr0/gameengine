// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_LIGHT_MANAGER_H_
#define COMET_RENDER_LIGHT_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_container.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/handle.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/rendering/type/light.h"

namespace comet {
namespace rendering {
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
                                            memory::kEngineMemoryTagRendering};
  Array<Light> lights_{};
  frame::DoubleFrameOrderedSet<LightHandle>* new_light_handles_{nullptr};
  frame::DoubleFrameOrderedSet<LightHandle>* destroyed_light_handles_{nullptr};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_LIGHT_MANAGER_H_