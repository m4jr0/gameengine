// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "scene_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/animation/animation_manager.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/entity/entity_event.h"
#include "comet/entity/entity_manager.h"
#include "comet/entity/factory/entity_factory_manager.h"
#include "comet/event/event_manager.h"
#include "comet/physics/component/transform_component.h"
#include "comet/physics/transform.h"
#include "comet/resource/resource.h"
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
  event::EventManager::Get().Register(COMET_EVENT_BIND_FUNCTION(OnEvent),
                                      entity::ModelLoadedEvent::kStaticType_);
}

void SceneManager::LoadScene() {
  // TODO(m4jr0): Load scene properly from a file or something.
  // Tags: scene
  LoadTmp();
}

usize SceneManager::GetExpectedEntityCount() const {
  // TODO(m4jr0): Be smarter.
  return 10000;
}

void SceneManager::OnEvent(const event::Event& event) {
  if (event.GetType() == SceneLoadRequestEvent::kStaticType_) {
    LoadScene();
  } else if (event.GetType() == entity::ModelLoadedEvent::kStaticType_) {
    HandleLoadedModelTmp(
        static_cast<const entity::ModelLoadedEvent&>(event).GetEntityId());
  }
}

void SceneManager::LoadTmp() {
  auto* model_handler{entity::EntityFactoryManager::Get().GetModel()};

  constexpr auto kIsEveLoaded{true};
  constexpr auto kIsVampireLoaded{true};
  constexpr auto kIsSponzaLoaded{true};
  constexpr auto kLifeSpan{resource::ResourceLifeSpan::Scene};

  if (kIsEveLoaded) {
    character_eve_id_tmp_ = model_handler->GenerateSkeletal(
        COMET_CTSTRING_VIEW("models/eve/eve.gltf"), kLifeSpan);
    ++models_to_load_count_;
  }

  if (kIsVampireLoaded) {
    character_vampire_id_tmp_ = model_handler->GenerateSkeletal(
        COMET_CTSTRING_VIEW("models/dancing_vampire/dancing_vampire.dae"),
        kLifeSpan);
    ++models_to_load_count_;
  }

  if (kIsSponzaLoaded) {
    sponza_id_tmp_ = model_handler->GenerateStatic(
        COMET_CTSTRING_VIEW("models/sponza/sponza.obj"), kLifeSpan);
    ++models_to_load_count_;
  }
}

void SceneManager::HandleLoadedModelTmp(entity::EntityId entity_id) {
  if (entity_id != character_eve_id_tmp_ &&
      entity_id != character_vampire_id_tmp_ && entity_id != sponza_id_tmp_) {
    return;
  }

  ++loaded_model_count_tmp_;

  if (loaded_model_count_tmp_ < models_to_load_count_) {
    return;
  }

  auto job_descr{job::GenerateJobDescr(
      job::JobPriority::Normal,
      [](job::JobParamsHandle) {
        auto& entity_manager{entity::EntityManager::Get()};
        auto& animation_manager{animation::AnimationManager::Get()};
        auto& scene_manager{SceneManager::Get()};
        entity_manager.WaitForEntityUpdates();

        if (entity_manager.IsEntity(scene_manager.character_eve_id_tmp_)) {
          auto* character_eve_transform{
              entity_manager.GetComponent<physics::TransformComponent>(
                  scene_manager.character_eve_id_tmp_)};

          if (character_eve_transform != nullptr) {
            constexpr auto kCharacterEveScaleTransform{1500.0f};
            physics::ScaleLocal(character_eve_transform,
                                kCharacterEveScaleTransform);

            animation_manager.Play(scene_manager.character_eve_id_tmp_,
                                   L"models/eve/eve.gltf|idle");
          }
        }

        if (entity_manager.IsEntity(scene_manager.character_vampire_id_tmp_)) {
          auto* character_vampire_transform{
              entity_manager.GetComponent<physics::TransformComponent>(
                  scene_manager.character_vampire_id_tmp_)};

          if (character_vampire_transform != nullptr) {
            constexpr auto kCharacterVampireScaleTransform{1000.0f};
            physics::ScaleLocal(character_vampire_transform,
                                kCharacterVampireScaleTransform);
            constexpr math::Vec3 kCharacterVampireTranslationTransform{
                -3.0f, .0f, .0f};
            physics::TranslateLocal(character_vampire_transform,
                                    kCharacterVampireTranslationTransform);

            animation_manager.Play(
                scene_manager.character_vampire_id_tmp_,
                "models/dancing_vampire/dancing_vampire.dae|Hips", 1.0f, true);
          }
        }

        if (entity_manager.IsEntity(scene_manager.sponza_id_tmp_)) {
          auto* sponza_transform{
              entity_manager.GetComponent<physics::TransformComponent>(
                  scene_manager.sponza_id_tmp_)};

          if (sponza_transform != nullptr) {
            constexpr auto kSponzaScaleFactor{0.17f};
            physics::ScaleLocal(sponza_transform, kSponzaScaleFactor);
          }
        }

        event::EventManager::Get().FireEvent<SceneLoadedEvent>();
      },
      nullptr, job::JobStackSize::Normal, nullptr, "loading_entities")};

  job::Scheduler::Get().Kick(job_descr);
}
}  // namespace scene
}  // namespace comet
