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

void LightManager::Initialize() {
  Manager::Initialize();
  allocator_.Initialize();
  lights_ = Array<Light>{&allocator_};
  new_light_ids_ = COMET_DOUBLE_FRAME_ORDERED_SET(LightId);
}

void LightManager::Shutdown() {
  lights_.Destroy();
  light_id_handler_.Shutdown();
  allocator_.Destroy();
  Manager::Shutdown();
}

void LightManager::Update(frame::FramePacket* packet) {
  COMET_PROFILE("LightManager::Update");
  SyncFromTransforms();
  EmitFramePacketChanges(packet);
  constexpr usize kNewLightCapacity{16};
  new_light_ids_ = COMET_DOUBLE_FRAME_ORDERED_SET(LightId);
  new_light_ids_->Reserve(kNewLightCapacity);
}

LightId LightManager::Generate(const LightDescr& descr) {
  auto id{static_cast<LightId>(light_id_handler_.Generate())};
  auto index{gid::GetIndex(id)};
  lights_.Resize(index + 1);

  lights_[index] = {
      .is_alive = true,
      .is_dirty = true,
      .entity_id = descr.entity_id,
      .id = id,
      .props = descr.props,
      .shadow = descr.shadow,
  };

  new_light_ids_->Add(id);
  return id;
}

void LightManager::Destroy(LightId id) {
  auto* light{Get(id)};
  light->is_alive = false;
  light->is_dirty = true;
}

void LightManager::SetProperties(LightId id, const LightProperties& props) {
  auto* light{Get(id)};
  light->props = props;
  light->is_dirty = true;
}

void LightManager::SetShadow(LightId id, const LightShadow& shadow) {
  auto* light{Get(id)};
  light->shadow = shadow;
  light->is_dirty = true;
}

void LightManager::SetColor(LightId id, const math::Vec3& color) {
  auto* light{Get(id)};
  light->props.color = color;
  light->is_dirty = true;
}

void LightManager::SetIntensity(LightId id, f32 intensity) {
  auto* light{Get(id)};
  light->props.intensity = intensity;
  light->is_dirty = true;
}

void LightManager::SetRange(LightId id, f32 range) {
  auto* light{Get(id)};
  light->props.range = range;
  light->is_dirty = true;
}

void LightManager::SetDirection(LightId id, const math::Vec3& direction) {
  auto* light{Get(id)};
  light->props.direction = direction;
  light->is_dirty = true;
}

bool LightManager::IsNew(LightId id) const noexcept {
  return new_light_ids_ != nullptr && new_light_ids_->IsContained(id);
}

void LightManager::SyncFromTransforms() {
  COMET_PROFILE("LightManager::SyncFromTransforms");
  auto& entity_manager{entity::EntityManager::Get()};

  for (auto& light : lights_) {
    if (!light.is_alive || light.entity_id == entity::kInvalidEntityId) {
      continue;
    }

    auto* transform{entity_manager.GetComponent<physics::TransformComponent>(
        light.entity_id)};

    if (transform == nullptr) {
      continue;
    }

    auto new_pos{ExtractPosition(transform->global)};
    auto new_dir{ExtractForward(transform->global)};

    // replace with epsilon compare later
    if (new_pos != light.props.position || new_dir != light.props.direction) {
      light.props.position = new_pos;
      light.props.direction = new_dir;
      light.is_dirty = true;
    }
  }
}

void LightManager::EmitFramePacketChanges(frame::FramePacket* packet) {
  COMET_PROFILE("LightManager::EmitFramePacketChanges");
  frame::FrameArray<LightId> dead_ids{};

  for (auto& light : lights_) {
    if (!light.is_alive) {
      dead_ids.PushBack(light.id);
      continue;
    }

    if (light.is_dirty) {
      if (IsNew(light.id)) {
        packet->RegisterNewLight(light.id, &light.props, &light.shadow);
      } else {
        packet->RegisterDirtyLight(light.id, &light.props, &light.shadow);
      }

      light.is_dirty = false;
    }
  }

  for (auto id : dead_ids) {
    auto index{gid::GetIndex(id)};
    packet->RegisterRemovedLight(id);
    lights_[index] = {};
    light_id_handler_.Destroy(id);
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

Light* LightManager::Get(LightId id) {
  COMET_ASSERT(light_id_handler_.IsAlive(id), "Light #", id, " is not alive!");
  auto& light{lights_[gid::GetIndex(id)]};
  return &light;
}

const Light* LightManager::Get(LightId id) const {
  COMET_ASSERT(light_id_handler_.IsAlive(id), "Light #", id, " is not alive!");
  auto& light{lights_[gid::GetIndex(id)]};
  return &light;
}
}  // namespace rendering
}  // namespace comet
