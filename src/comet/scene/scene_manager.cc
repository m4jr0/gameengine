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
#include "comet/animation/animation_set.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/entity/entity_event.h"
#include "comet/entity/entity_manager.h"
#include "comet/entity/factory/entity_factory_manager.h"
#include "comet/environment/environment_manager.h"
#include "comet/math/geometry.h"
#include "comet/physics/component/transform_component.h"
#include "comet/physics/transform.h"
#include "comet/rendering/light_manager.h"
#include "comet/rendering/type/rendering_light_type.h"
#include "comet/resource/type/resource_common_type.h"
#include "comet/scene/scene_event.h"

namespace comet {
namespace scene {
SceneManager& SceneManager::Get() {
  static SceneManager singleton{};
  return singleton;
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

void SceneManager::OnInitialize() { RegisterEvents(); }

void SceneManager::OnShutdown() { UnregisterEvents(); }

void SceneManager::OnEvent(const event::Event& event) {
  if (event.GetType() == SceneLoadRequestEvent::kStaticType_) {
    LoadScene();
  } else if (event.GetType() == entity::ModelLoadedEvent::kStaticType_) {
    HandleLoadedModelTmp(
        static_cast<const entity::ModelLoadedEvent&>(event).GetEntityId());
  }
}

void SceneManager::RegisterEvents() {
  auto& event_manager{event::EventManager::Get()};
  const auto event_function{
      [this](const event::Event& event) { OnEvent(event); }};

  scene_load_request_listener_id_ = event_manager.Register(
      event_function, SceneLoadRequestEvent::kStaticType_);
  COMET_ASSERT(
      scene_load_request_listener_id_ != event::kInvalidEventListenerId,
      "SceneManager::RegisterEvents",
      "scene load request listener registration failed");

  model_loaded_listener_id_ = event_manager.Register(
      event_function, entity::ModelLoadedEvent::kStaticType_);
  COMET_ASSERT(model_loaded_listener_id_ != event::kInvalidEventListenerId,
               "SceneManager::RegisterEvents",
               "model loaded listener registration failed");
}

void SceneManager::UnregisterEvents() {
  auto& event_manager{event::EventManager::Get()};

  if (scene_load_request_listener_id_ != event::kInvalidEventListenerId) {
    event_manager.Unregister(scene_load_request_listener_id_);
    scene_load_request_listener_id_ = event::kInvalidEventListenerId;
  }

  if (model_loaded_listener_id_ != event::kInvalidEventListenerId) {
    event_manager.Unregister(model_loaded_listener_id_);
    model_loaded_listener_id_ = event::kInvalidEventListenerId;
  }
}

void SceneManager::LoadTmp() {
  models_to_load_count_ = 0;
  loaded_model_count_tmp_ = 0;
  character_eve_id_tmp_ = entity::kInvalidEntityId;
  character_vampire_id_tmp_ = entity::kInvalidEntityId;
  sponza_id_tmp_ = entity::kInvalidEntityId;

  environment::EnvironmentManager::Get().SetAzimuthOffsetRadians(
      math::ConvertToRadians(-180.0f));

  auto& factory_manager{entity::EntityFactoryManager::Get()};

  auto* model_handler{factory_manager.GetModel()};
  COMET_ASSERT(model_handler != nullptr, "SceneManager::LoadTmp",
               "model factory handler is null");

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
        COMET_CTSTRING_VIEW("models/sponza/Sponza.gltf"), kLifeSpan);
    ++models_to_load_count_;
  }

  constexpr usize kSpotLightCount{4};
  constexpr f32 kSpotlightHeight{1.4f};
  constexpr bool kSpotlightHasShadows{false};

  constexpr StaticArray<math::Vec3, kSpotLightCount> kSpotLightOffsets{
      math::Vec3{6.63f, kSpotlightHeight, 2.0f},
      math::Vec3{-8.43f, kSpotlightHeight, 2.0f},
      math::Vec3{6.63f, kSpotlightHeight, -3.0f},
      math::Vec3{-8.43f, kSpotlightHeight, -3.0f}};

  for (usize i{0}; i < kSpotLightCount; ++i) {
    const auto& offset{kSpotLightOffsets[i]};

    rendering::LightManager::Get().Generate({
        .props =
            {
                .type = rendering::LightType::Spot,
                .position = offset,
                .direction = {.0f, -1.0f, .0f},
                .color = math::Vec3{1.0f, .55f, .18f},
                .intensity = 6.0f,
                .range = 12.0f,
                .inner_angle = .35f,
                .outer_angle = .40f,
            },
        .shadow =
            {
                .is_enabled = kSpotlightHasShadows,
                .max_distance = 12.0f,
                .bias_constant = .0001f,
                .bias_slope = .001f,
            },
    });
  }
}

void SceneManager::HandleLoadedModelTmp(entity::EntityId entity_id) {
  COMET_ASSERT(models_to_load_count_ != 0, "SceneManager::HandleLoadedModelTmp",
               "no models were scheduled for loading");

  if (entity_id != character_eve_id_tmp_ &&
      entity_id != character_vampire_id_tmp_ && entity_id != sponza_id_tmp_) {
    return;
  }

  ++loaded_model_count_tmp_;
  COMET_ASSERT(models_to_load_count_ >= loaded_model_count_tmp_,
               "SceneManager::HandleLoadedModelTmp",
               "loaded model count exceeds expected count",
               "loaded_model_count", loaded_model_count_tmp_,
               "models_to_load_count", models_to_load_count_);

  if (loaded_model_count_tmp_ < models_to_load_count_) {
    return;
  }

  const auto job_descr{job::GenerateJobDescr(
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
            constexpr auto kCharacterEveScaleTransform{153.0f};
            physics::ScaleLocal(character_eve_transform,
                                kCharacterEveScaleTransform);

            constexpr math::Vec3 kCharacterEveTranslationTransform{1.0f, .0f,
                                                                   .0f};
            physics::TranslateLocal(character_eve_transform,
                                    kCharacterEveTranslationTransform);

            constexpr auto kCharacterEveRotateTransform{
                math::ConvertToRadians(40.f)};
            physics::RotateLocal(character_eve_transform,
                                 kCharacterEveRotateTransform,
                                 {.0f, 1.0f, .0f});

            animation::AnimationSet eve_anims{&scene_manager.tmp_allocator_, 3};

            const auto idle_anim{
                animation::GenerateAnimationClipId("models/eve/eve.gltf|idle")};
            COMET_ASSERT(idle_anim, "SceneManager::HandleLoadedModelTmp",
                         "eve idle animation clip id is invalid");
            eve_anims.Set("idle", idle_anim);

            const auto walk_anim{
                animation::GenerateAnimationClipId("models/eve/eve.gltf|walk")};
            COMET_ASSERT(walk_anim, "SceneManager::HandleLoadedModelTmp",
                         "eve walk animation clip id is invalid");
            eve_anims.Set("walk", walk_anim);

            const auto run_anim{
                animation::GenerateAnimationClipId("models/eve/eve.gltf|run")};
            COMET_ASSERT(run_anim, "SceneManager::HandleLoadedModelTmp",
                         "eve run animation clip id is invalid");
            eve_anims.Set("run", run_anim);

            animation_manager.Play(scene_manager.character_eve_id_tmp_,
                                   eve_anims.Get("idle"), 1.0f, true);
          }
        }

        if (entity_manager.IsEntity(scene_manager.character_vampire_id_tmp_)) {
          auto* character_vampire_transform{
              entity_manager.GetComponent<physics::TransformComponent>(
                  scene_manager.character_vampire_id_tmp_)};

          if (character_vampire_transform != nullptr) {
            constexpr auto kCharacterVampireScaleTransform{102.0f};
            physics::ScaleLocal(character_vampire_transform,
                                kCharacterVampireScaleTransform);

            constexpr math::Vec3 kCharacterVampireTranslationTransform{
                -1.0f, .0f, .0f};
            physics::TranslateLocal(character_vampire_transform,
                                    kCharacterVampireTranslationTransform);

            constexpr auto kCharacterVampireRotateTransform{
                math::ConvertToRadians(70.0f)};
            physics::RotateLocal(character_vampire_transform,
                                 kCharacterVampireRotateTransform,
                                 {.0f, 1.0f, .0f});

            animation::AnimationSet vampire_anims{&scene_manager.tmp_allocator_,
                                                  1};
            const auto dance_anim{animation::GenerateAnimationClipId(
                "models/dancing_vampire/dancing_vampire.dae|Hips")};
            COMET_ASSERT(dance_anim, "SceneManager::HandleLoadedModelTmp",
                         "vampire dance animation clip id is invalid");

            vampire_anims.Set("dance", dance_anim);

            animation_manager.Play(scene_manager.character_vampire_id_tmp_,
                                   vampire_anims.Get("dance"), 1.0f, true);
          }
        }

        if (entity_manager.IsEntity(scene_manager.sponza_id_tmp_)) {
          auto* sponza_transform{
              entity_manager.GetComponent<physics::TransformComponent>(
                  scene_manager.sponza_id_tmp_)};

          if (sponza_transform != nullptr) {
            constexpr auto kSponzaScaleFactor{1.7f};
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