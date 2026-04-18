// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "light_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/type/ordered_set.h"
#include "comet/entity/entity_manager.h"
#include "comet/profiler/profiler.h"

namespace comet {
namespace rendering {
LightManager& LightManager::Get() {
  static LightManager singleton{};
  return singleton;
}

void LightManager::Update(frame::FramePacket* packet) {
  COMET_PROFILE("LightManager::Update");
  SyncFromTransforms();
  EmitFramePacketChanges(packet);

  constexpr usize kCapacity{16};
  new_light_handles_ = COMET_DOUBLE_FRAME_ORDERED_SET(LightHandle);
  new_light_handles_->Reserve(kCapacity);
  destroyed_light_handles_ = COMET_DOUBLE_FRAME_ORDERED_SET(LightHandle);
  destroyed_light_handles_->Reserve(kCapacity);
}

LightHandle LightManager::Generate(const LightDescr& descr) {
  const auto handle{light_pool_.Generate()};
  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= lights_.GetSize()) {
    lights_.Resize(index + 1);
  }

  lights_[index] = {
      .is_dirty = true,
      .entity_id = descr.entity_id,
      .handle = handle,
      .props = descr.props,
      .shadow = descr.shadow,
  };

  new_light_handles_->Add(handle);
  return handle;
}

void LightManager::Destroy(LightHandle handle) {
  if (IsShutdownPending()) {
    const auto index{static_cast<usize>(handle.GetIndex())};
    COMET_ASSERT(index < lights_.GetSize(),
                 "Light handle index out of bounds!");

    lights_[index] = {};
    light_pool_.Destroy(handle);
    return;
  }

  auto* light{Get(handle)};
  light->is_dirty = false;

  COMET_ASSERT(destroyed_light_handles_ != nullptr,
               "Destroyed light handle set is null!");
  COMET_ASSERT(new_light_handles_ != nullptr, "New light handle set is null!");

  destroyed_light_handles_->Add(handle);
  new_light_handles_->Remove(handle);
}

void LightManager::SetProperties(LightHandle handle,
                                 const LightProperties& props) {
  auto* light{Get(handle)};
  light->props = props;
  light->is_dirty = true;
}

void LightManager::SetShadow(LightHandle handle, const LightShadow& shadow) {
  auto* light{Get(handle)};
  light->shadow = shadow;
  light->is_dirty = true;
}

void LightManager::SetColor(LightHandle handle, const math::Vec3& color) {
  auto* light{Get(handle)};
  light->props.color = color;
  light->is_dirty = true;
}

void LightManager::SetIntensity(LightHandle handle, f32 intensity) {
  auto* light{Get(handle)};
  light->props.intensity = intensity;
  light->is_dirty = true;
}

void LightManager::SetRange(LightHandle handle, f32 range) {
  auto* light{Get(handle)};
  light->props.range = range;
  light->is_dirty = true;
}

void LightManager::SetDirection(LightHandle handle,
                                const math::Vec3& direction) {
  auto* light{Get(handle)};
  light->props.direction = direction;
  light->is_dirty = true;
}

void LightManager::OnInitialize() {
  allocator_.Initialize();
  lights_ = Array<Light>{&allocator_};

  new_light_handles_ = COMET_DOUBLE_FRAME_ORDERED_SET(LightHandle);
  destroyed_light_handles_ = COMET_DOUBLE_FRAME_ORDERED_SET(LightHandle);

  constexpr usize kCapacity{16};
  new_light_handles_->Reserve(kCapacity);
  destroyed_light_handles_->Reserve(kCapacity);
}

void LightManager::OnShutdown() {
  lights_.Destroy();
  light_pool_.Destroy();
  allocator_.Destroy();
}

bool LightManager::IsNew(LightHandle handle) const noexcept {
  return new_light_handles_ != nullptr &&
         new_light_handles_->IsContained(handle);
}

void LightManager::SyncFromTransforms() {
  COMET_PROFILE("LightManager::SyncFromTransforms");
  auto& entity_manager{entity::EntityManager::Get()};

  for (auto& light : lights_) {
    if (!light.handle || !light_pool_.IsAlive(light.handle) ||
        light.entity_id == entity::kInvalidEntityId) {
      continue;
    }

    if (destroyed_light_handles_ != nullptr &&
        destroyed_light_handles_->IsContained(light.handle)) {
      continue;
    }

    auto* transform{entity_manager.GetComponent<physics::TransformComponent>(
        light.entity_id)};

    if (transform == nullptr) {
      continue;
    }

    const auto new_pos{ExtractPosition(transform->global)};
    const auto new_dir{ExtractForward(transform->global)};

    // Replace with epsilon compare later.
    if (new_pos != light.props.position || new_dir != light.props.direction) {
      light.props.position = new_pos;
      light.props.direction = new_dir;
      light.is_dirty = true;
    }
  }
}

void LightManager::EmitFramePacketChanges(frame::FramePacket* packet) {
  COMET_PROFILE("LightManager::EmitFramePacketChanges");

  for (auto& light : lights_) {
    if (!light.handle || !light_pool_.IsAlive(light.handle)) {
      continue;
    }

    if (destroyed_light_handles_ != nullptr &&
        destroyed_light_handles_->IsContained(light.handle)) {
      continue;
    }

    if (light.is_dirty) {
      if (IsNew(light.handle)) {
        packet->RegisterNewLight(light.handle, &light.props, &light.shadow);
      } else {
        packet->RegisterDirtyLight(light.handle, &light.props, &light.shadow);
      }

      light.is_dirty = false;
    }
  }

  if (destroyed_light_handles_ == nullptr) {
    return;
  }

  for (const auto handle : *destroyed_light_handles_) {
    if (!light_pool_.IsAlive(handle)) {
      continue;
    }

    const auto index{static_cast<usize>(handle.GetIndex())};
    packet->RegisterRemovedLight(handle);
    lights_[index] = {};
    light_pool_.Destroy(handle);
  }
}

math::Vec3 LightManager::ExtractPosition(const math::Mat4& transform) const {
  return math::Vec3{transform[3][0], transform[3][1], transform[3][2]};
}

math::Vec3 LightManager::ExtractForward(const math::Mat4& transform) const {
  // Convention: forward = -Z axis.
  math::Vec3 forward{-transform[2][0], -transform[2][1], -transform[2][2]};
  math::Normalize(forward);
  return forward;
}

Light* LightManager::Get(LightHandle handle) {
  COMET_ASSERT(light_pool_.IsAlive(handle), "Light ", handle, " is not alive!");
  const auto index{static_cast<usize>(handle.GetIndex())};
  COMET_ASSERT(index < lights_.GetSize(), "Light handle index out of bounds!");
  return &lights_[index];
}

const Light* LightManager::Get(LightHandle handle) const {
  COMET_ASSERT(light_pool_.IsAlive(handle), "Light ", handle, " is not alive!");
  const auto index{static_cast<usize>(handle.GetIndex())};
  COMET_ASSERT(index < lights_.GetSize(), "Light handle index out of bounds!");
  return &lights_[index];
}
}  // namespace rendering
}  // namespace comet