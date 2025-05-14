// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "scene_manager.h"

#include "comet/animation/animation_manager.h"
#include "comet/core/concurrency/fiber/fiber_utils.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
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
  static entity::EntityId character_eve_id{entity::kInvalidEntityId};
  static entity::EntityId character_vampire_id{entity::kInvalidEntityId};
  static entity::EntityId sponza_id{entity::kInvalidEntityId};

  constexpr auto kIsEveLoaded{true};
  constexpr auto kIsVampireLoaded{true};
  constexpr auto kIsSponzaLoaded{true};

  if (kIsEveLoaded) {
    character_eve_id =
        entity::EntityFactoryManager::Get().GetModel()->GenerateSkeletal(
            COMET_CTSTRING_VIEW("models/eve/eve.gltf"), packet);
  }

  if (kIsVampireLoaded) {
    character_vampire_id =
        entity::EntityFactoryManager::Get().GetModel()->GenerateSkeletal(
            COMET_CTSTRING_VIEW("models/dancing_vampire/dancing_vampire.dae"),
            packet);
  }

  if (kIsSponzaLoaded) {
    sponza_id = entity::EntityFactoryManager::Get().GetModel()->GenerateStatic(
        COMET_CTSTRING_VIEW("models/sponza/sponza.obj"), packet);
  }

  auto job_descr{job::GenerateJobDescr(
      job::JobPriority::Normal,
      [](job::JobParamsHandle) {
        fiber::WaitForEntityUpdates();

        auto& entity_manager{entity::EntityManager::Get()};
        auto& animation_manager{animation::AnimationManager::Get()};

        if (entity_manager.IsEntity(character_eve_id)) {
          auto* character_eve_transform{
              entity_manager.GetComponent<physics::TransformComponent>(
                  character_eve_id)};

          constexpr auto kCharacterEveScaleTransform{1500.0f};
          physics::ScaleLocal(character_eve_transform,
                              kCharacterEveScaleTransform);

          animation_manager.Play(character_eve_id, L"models/eve/eve.gltf|idle");
        }

        if (entity_manager.IsEntity(character_vampire_id)) {
          auto* character_vampire_transform{
              entity_manager.GetComponent<physics::TransformComponent>(
                  character_vampire_id)};

          constexpr auto kCharacterVampireScaleTransform{1000.0f};
          physics::ScaleLocal(character_vampire_transform,
                              kCharacterVampireScaleTransform);
          constexpr math::Vec3 kCharacterVampireTranslationTransform{-3.0f, .0f,
                                                                     .0f};
          physics::TranslateLocal(character_vampire_transform,
                                  kCharacterVampireTranslationTransform);

          animation_manager.Play(
              character_vampire_id,
              "models/dancing_vampire/dancing_vampire.dae|Hips", 1.0f, true);
        }

        if (entity_manager.IsEntity(sponza_id)) {
          auto* sponza_transform{
              entity_manager.GetComponent<physics::TransformComponent>(
                  sponza_id)};

          constexpr auto kSponzaScaleFactor{0.17f};
          physics::ScaleLocal(sponza_transform, kSponzaScaleFactor);
        }

        event::EventManager::Get().FireEvent<SceneLoadedEvent>();
      },
      nullptr, job::JobStackSize::Normal, nullptr, "loading_entities")};

  auto& scheduler{job::Scheduler::Get()};
  scheduler.Kick(job_descr);
}
}  // namespace scene
}  // namespace comet
