// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_LIGHT_LIGHT_MANAGER_H_
#define COMET_COMET_RENDERING_LIGHT_LIGHT_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/array.h"
#include "comet/rendering/light/light_common.h"

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
  virtual ~LightManager() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update(frame::FramePacket* packet);

  LightId Generate(const LightDescr& descr);
  void Destroy(LightId id);

  void SetProperties(LightId id, const LightProperties& props);
  void SetShadow(LightId id, const LightShadow& shadow);

  void SetColor(LightId id, const math::Vec3& color);
  void SetIntensity(LightId id, f32 intensity);
  void SetRange(LightId id, f32 range);
  void SetDirection(LightId id, const math::Vec3& direction);

 private:
  bool IsNew(LightId id) const noexcept;
  void SyncFromTransforms();
  void EmitFramePacketChanges(frame::FramePacket* packet);

  math::Vec3 ExtractPosition(const math::Mat4& transform) const;
  math::Vec3 ExtractForward(const math::Mat4& transform) const;

  Light* Get(LightId id);
  const Light* Get(LightId id) const;

  gid::BreedHandler light_id_handler_{};
  memory::FiberFreeListAllocator allocator_{sizeof(u32), 256,
                                            memory::kEngineMemoryTagRendering};
  Array<Light> lights_{};
  frame::DoubleFrameOrderedSet<LightId>* new_light_ids_{nullptr};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_LIGHT_LIGHT_MANAGER_H_
