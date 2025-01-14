// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "scene_manager.h"

#include "comet/core/concurrency/fiber/fiber_utils.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/entity/entity_manager.h"
#include "comet/entity/factory/entity_factory_manager.h"
#include "comet/event/event_manager.h"
#include "comet/physics/component/transform_component.h"
#include "comet/physics/transform.h"
#include "comet/scene/scene_event.h"

namespace comet {
namespace scene {
SceneManager& SceneManager::Get() {
  static SceneManager singleton{};
  return singleton;
}

void SceneManager::Initialize() {
  Manager::Initialize();
  event::EventManager::Get().Register(COMET_EVENT_BIND_FUNCTION(OnEvent),
                                      SceneLoadRequestEvent::kStaticType_);
}

void SceneManager::LoadScene(frame::FramePacket* packet) {
  // TODO(m4jr0): Load scene properly from a file or something.
  // Tags: scene
  LoadTmp(packet);
}

usize SceneManager::GetExpectedEntityCount() const {
  // TODO(m4jr0): Be smarter.
  return 10000;
}

void SceneManager::OnEvent(const event::Event& event) {
  if (event.GetType() == SceneLoadRequestEvent::kStaticType_) {
    const auto& request_event{
        reinterpret_cast<const SceneLoadRequestEvent&>(event)};
    LoadScene(request_event.GetFramePacket());
  }
}

void SceneManager::LoadTmp(frame::FramePacket* packet) {
  static entity::EntityId character_id;
  static entity::EntityId sponza_id;

  character_id =
      entity::EntityFactoryManager::Get().GetModel()->GenerateSkeletal(
          COMET_CTSTRING_VIEW("models/kate/kate.fbx"), packet);

  sponza_id = entity::EntityFactoryManager::Get().GetModel()->GenerateStatic(
      COMET_CTSTRING_VIEW("models/sponza/sponza.obj"), packet);

  auto job_descr{job::GenerateJobDescr(
      job::JobPriority::Normal,
      [](job::JobParamsHandle) {
        fiber::WaitForEntityUpdates();

        auto* character_transform{
            entity::EntityManager::Get()
                .GetComponent<physics::TransformComponent>(character_id)};

        constexpr auto kCharacterScaleFactor{0.15f};
        physics::ScaleLocal(character_transform, kCharacterScaleFactor);

        auto* sponza_transform{
            entity::EntityManager::Get()
                .GetComponent<physics::TransformComponent>(sponza_id)};

        constexpr auto kSponzaScaleFactor{0.17f};
        physics::ScaleLocal(sponza_transform, kSponzaScaleFactor);

        event::EventManager::Get().FireEvent<SceneLoadedEvent>();
      },
      nullptr, job::JobStackSize::Normal, nullptr, "loading_entities")};

  auto& scheduler{job::Scheduler::Get()};
  scheduler.Kick(job_descr);
}
}  // namespace scene
}  // namespace comet
