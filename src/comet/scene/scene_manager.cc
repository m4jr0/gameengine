// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "scene_manager.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/animation/animation_manager.h"
#include "comet/animation/animation_set.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/frame/frame_manager.h"
#include "comet/entity/component.h"
#include "comet/entity/entity_changes_fence.h"
#include "comet/entity/entity_event.h"
#include "comet/entity/entity_manager.h"
#include "comet/entity/factory/entity_factory_manager.h"
#include "comet/entity/type/entity_id.h"
#include "comet/entity/utils/entity_id_utils.h"
#include "comet/environment/environment_manager.h"
#include "comet/math/geometry.h"
#include "comet/physics/component/transform_component.h"
#include "comet/physics/transform.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/camera_manager.h"
#include "comet/rendering/light_manager.h"
#include "comet/rendering/type/light.h"
#include "comet/resource/type/common.h"
#include "comet/scene/scene_event.h"
#include "comet/time/time_manager.h"

namespace comet {
namespace scene {
namespace internal {
// Temporary scene/testbed state.
// TODO(m4jr0): Replace this with proper scene loading, scenario
// configuration,
// and dedicated ECS regression tests.
constexpr bool kIsRenderProxyStressEnabled{false};
constexpr bool kIsMeshStressEnabled{false};
constexpr bool kIsComponentStressEnabled{false};

constexpr bool kIsCubeLoaded{false};
constexpr bool kIsEveLoaded{true};
constexpr bool kIsVampireLoaded{true};
constexpr bool kIsSponzaLoaded{true};

constexpr auto kSceneLifeSpan{resource::ResourceLifeSpan::Scene};

constexpr usize kMaxSceneModelJobs{3};
constexpr usize kRenderProxyGridX{4};
constexpr usize kRenderProxyGridZ{4};
constexpr usize kMaxRenderProxyJobs{kRenderProxyGridX * kRenderProxyGridZ};

constexpr f32 kRenderProxySpacing{2.5f};
constexpr auto kRenderProxyModelPath{
    COMET_CTSTRING_VIEW("models/eve/eve.gltf")};

enum class AsyncModelKind {
  Static,
  Skeletal,
};

enum class AsyncGroupState {
  Idle,
  AwaitingModelLoaded,
  WaitingForEntityChanges,
  Finalizing,
  Loaded,
  Destroying,
};

struct AsyncModelGroup {
  AsyncGroupState state{AsyncGroupState::Idle};
  usize job_count{0};

  // Jobs returned from factory calls.
  std::atomic<usize> completed_count{0};

  // ModelLoadedEvent received after geometry registration.
  std::atomic<usize> loaded_count{0};

  entity::EntityChangesFence entity_fence{};
};

struct AsyncModelGroupFence {
  AsyncModelGroup* group{nullptr};

  bool Poll() const noexcept {
    return group != nullptr &&
           group->loaded_count.load(std::memory_order_acquire) >=
               group->job_count;
  }
};

struct AsyncModelJob {
  AsyncModelKind kind{AsyncModelKind::Static};
  CTStringView path{};
  resource::ResourceLifeSpan life_span{kSceneLifeSpan};
  entity::EntityId* out_entity_id{nullptr};
  std::atomic<usize>* completed_count{nullptr};
};

static void ResetGroup(AsyncModelGroup& group) {
  group.state = AsyncGroupState::Idle;
  group.job_count = 0;
  group.completed_count.store(0, std::memory_order_relaxed);
  group.loaded_count.store(0, std::memory_order_relaxed);
  group.entity_fence.Reset();
}

memory::PlatformAllocator tmp_allocator{memory::kEngineMemoryTagResourceScene};

AsyncModelGroup scene_load_group_tmp{};
AsyncModelJob scene_model_jobs_tmp[kMaxSceneModelJobs]{};

AsyncModelGroup render_proxy_group_tmp{};
AsyncModelJob render_proxy_jobs_tmp[kMaxRenderProxyJobs]{};

entity::EntityId cube_id_tmp{entity::kInvalidEntityId};
entity::EntityId character_eve_id_tmp{entity::kInvalidEntityId};
entity::EntityId character_vampire_id_tmp{entity::kInvalidEntityId};
entity::EntityId sponza_id_tmp{entity::kInvalidEntityId};

Array<entity::EntityId> stress_ids_tmp{
    Array<entity::EntityId>::WithCapacity(&tmp_allocator, kMaxRenderProxyJobs)};
Array<math::Vec2> stress_pos_tmp{
    Array<math::Vec2>::WithCapacity(&tmp_allocator, kMaxRenderProxyJobs)};
Array<entity::EntityId> pending_scene_model_ids_tmp{
    Array<entity::EntityId>::WithCapacity(&tmp_allocator, kMaxSceneModelJobs)};

event::EventListenerId model_loaded_listener_id{event::kInvalidEventListenerId};

bool are_render_proxy_tmp_spawned{false};

f64 render_proxy_stress_timer{.0};
f64 mesh_stress_timer{.0};

struct TestPositionComponent {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  f32 x{0};
  f32 y{0};
  f32 z{0};
};

struct TestVelocityComponent {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  f32 x{0};
  f32 y{0};
  f32 z{0};
};

struct TestHealthComponent {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  s32 hp{100};
};

struct TestMarkerComponent {
  entity::EntityId entity_id{entity::kInvalidEntityId};
};

struct TestBigComponent {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  u64 payload[16]{};
};

static_assert(entity::is_component_v<TestPositionComponent>);
static_assert(entity::is_component_v<TestVelocityComponent>);
static_assert(entity::is_component_v<TestHealthComponent>);
static_assert(entity::is_component_v<TestMarkerComponent>);
static_assert(entity::is_component_v<TestBigComponent>);

static void KickModelJob(AsyncModelJob* job_params) {
  COMET_ASSERT(job_params != nullptr, "SceneManager::KickModelJob",
               "job params are null");

  const auto job_descr{job::GenerateJobDescr(
      job::JobPriority::Normal,
      [](job::JobParamsHandle params_handle) {
        auto* params{reinterpret_cast<AsyncModelJob*>(params_handle)};

        COMET_ASSERT(params != nullptr, "SceneManager::AsyncModelJob",
                     "job params are null");
        COMET_ASSERT(params->out_entity_id != nullptr,
                     "SceneManager::AsyncModelJob", "output entity id is null");
        COMET_ASSERT(params->completed_count != nullptr,
                     "SceneManager::AsyncModelJob", "completed count is null");

        auto* model_handler{entity::EntityFactoryManager::Get().GetModel()};
        COMET_ASSERT(model_handler != nullptr, "SceneManager::AsyncModelJob",
                     "model factory handler is null");

        entity::EntityId entity_id{entity::kInvalidEntityId};

        switch (params->kind) {
          case AsyncModelKind::Static:
            entity_id =
                model_handler->GenerateStatic(params->path, params->life_span);
            break;

          case AsyncModelKind::Skeletal:
            entity_id = model_handler->GenerateSkeletal(params->path,
                                                        params->life_span);
            break;

          default:
            COMET_ASSERT(false, "SceneManager::AsyncModelJob",
                         "unsupported model kind");
            break;
        }

        *params->out_entity_id = entity_id;
        params->completed_count->fetch_add(1, std::memory_order_release);
      },
      job_params, job::JobStackSize::Large, nullptr, "async_model_load")};

  job::Scheduler::Get().Kick(job_descr);
}

static void GenerateLightsTmp() {
  COMET_PROFILE("SceneManager::GenerateLightsTmp");
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

static void AddSceneModelJob(AsyncModelKind kind, CTStringView path,
                             entity::EntityId* out_entity_id) {
  COMET_ASSERT(scene_load_group_tmp.job_count < kMaxSceneModelJobs,
               "SceneManager::AddSceneModelJob", "too many scene model jobs");

  auto& params{scene_model_jobs_tmp[scene_load_group_tmp.job_count++]};
  params.kind = kind;
  params.path = path;
  params.life_span = kSceneLifeSpan;
  params.out_entity_id = out_entity_id;
  params.completed_count = &scene_load_group_tmp.completed_count;
}

static void FinalizeSceneModelsTmp() {
  COMET_PROFILE("SceneManager::FinalizeSceneModelsTmp");
  auto& entity_manager{entity::EntityManager::Get()};
  auto& animation_manager{animation::AnimationManager::Get()};

  if (entity_manager.IsEntity(cube_id_tmp)) {
    auto* transform{
        entity_manager.GetComponent<physics::TransformComponent>(cube_id_tmp)};

    if (transform != nullptr) {
      physics::TranslateLocal(transform, math::Vec3{0.0f, 2.0f, 0.0f});
      physics::ScaleLocal(transform, 1.0f);
    }
  }

  if (entity_manager.IsEntity(character_eve_id_tmp)) {
    auto* transform{entity_manager.GetComponent<physics::TransformComponent>(
        character_eve_id_tmp)};

    if (transform != nullptr) {
      physics::ScaleLocal(transform, 153.0f);
      physics::TranslateLocal(transform, math::Vec3{1.0f, .0f, .0f});
      physics::RotateLocal(transform, math::ConvertToRadians(40.0f),
                           {.0f, 1.0f, .0f});

      animation::AnimationSet anims{&tmp_allocator, 3};
      anims.Set("idle",
                animation::GenerateAnimationClipId("models/eve/eve.gltf|idle"));
      anims.Set("walk",
                animation::GenerateAnimationClipId("models/eve/eve.gltf|walk"));
      anims.Set("run",
                animation::GenerateAnimationClipId("models/eve/eve.gltf|run"));

      animation_manager.Play(character_eve_id_tmp, anims.Get("idle"), 1.0f,
                             true);
    }
  }

  if (entity_manager.IsEntity(character_vampire_id_tmp)) {
    auto* transform{entity_manager.GetComponent<physics::TransformComponent>(
        character_vampire_id_tmp)};

    if (transform != nullptr) {
      physics::ScaleLocal(transform, 102.0f);
      physics::TranslateLocal(transform, math::Vec3{-1.0f, .0f, .0f});
      physics::RotateLocal(transform, math::ConvertToRadians(70.0f),
                           {.0f, 1.0f, .0f});

      animation::AnimationSet anims{&tmp_allocator, 1};
      anims.Set("dance",
                animation::GenerateAnimationClipId(
                    "models/dancing_vampire/dancing_vampire.dae|Hips"));

      animation_manager.Play(character_vampire_id_tmp, anims.Get("dance"), 1.0f,
                             true);
    }
  }

  if (entity_manager.IsEntity(sponza_id_tmp)) {
    auto* transform{entity_manager.GetComponent<physics::TransformComponent>(
        sponza_id_tmp)};

    if (transform != nullptr) {
      physics::ScaleLocal(transform, 1.7f);
    }
  }

  event::EventManager::Get().FireEvent<SceneLoadedEvent>();
}

static void StartSceneModelJobs() {
  COMET_PROFILE("SceneManager::StartSceneModelJobs");

  scene_load_group_tmp.state = AsyncGroupState::AwaitingModelLoaded;

  for (usize i{0}; i < scene_load_group_tmp.job_count; ++i) {
    KickModelJob(&scene_model_jobs_tmp[i]);
  }

  entity::ThenAfterEntityChanges(
      AsyncModelGroupFence{&scene_load_group_tmp}, [] {
        scene_load_group_tmp.state = AsyncGroupState::Finalizing;
        FinalizeSceneModelsTmp();
        scene_load_group_tmp.state = AsyncGroupState::Loaded;
      });
}

static void SetUpCameras() {
  auto& camera_manager{rendering::CameraManager::Get()};

  rendering::CameraPose sponza_camera_pose{
      .position = {4.491f, 1.285f, 1.995f},
      .rotation = {.831f, .0f, .55f, .01f},
  };

  camera_manager.SetResetPose(camera_manager.GetMainCamera(),
                              sponza_camera_pose);
  camera_manager.Reset(camera_manager.GetMainCamera());

#ifdef COMET_DEBUG
  camera_manager.SetResetPose(camera_manager.GetDebugCamera(),
                              sponza_camera_pose);
  camera_manager.Reset(camera_manager.GetDebugCamera());
#endif  // COMET_DEBUG
}

static void LoadSceneTmp() {
  COMET_PROFILE("SceneManager::LoadSceneTmp");
  ResetGroup(scene_load_group_tmp);
  ResetGroup(render_proxy_group_tmp);

  pending_scene_model_ids_tmp.Clear();

  SetUpCameras();

  character_eve_id_tmp = entity::kInvalidEntityId;
  character_vampire_id_tmp = entity::kInvalidEntityId;
  sponza_id_tmp = entity::kInvalidEntityId;

  stress_ids_tmp.Clear();
  stress_pos_tmp.Clear();
  are_render_proxy_tmp_spawned = false;

  environment::EnvironmentManager::Get().SetAzimuthOffsetRadians(
      math::ConvertToRadians(-180.0f));

  if constexpr (kIsCubeLoaded) {
    auto* primitive_handler{entity::EntityFactoryManager::Get().GetPrimitive()};

    COMET_ASSERT(primitive_handler != nullptr, "SceneManager::LoadSceneTmp",
                 "primitive handler is null");

    cube_id_tmp = primitive_handler->GenerateCube(1.0f, kSceneLifeSpan);
  }

  if constexpr (kIsEveLoaded) {
    AddSceneModelJob(AsyncModelKind::Skeletal,
                     COMET_CTSTRING_VIEW("models/eve/eve.gltf"),
                     &character_eve_id_tmp);
  }

  if constexpr (kIsVampireLoaded) {
    AddSceneModelJob(
        AsyncModelKind::Skeletal,
        COMET_CTSTRING_VIEW("models/dancing_vampire/dancing_vampire.dae"),
        &character_vampire_id_tmp);
  }

  if constexpr (kIsSponzaLoaded) {
    AddSceneModelJob(AsyncModelKind::Static,
                     COMET_CTSTRING_VIEW("models/sponza/Sponza.gltf"),
                     &sponza_id_tmp);
  }

  GenerateLightsTmp();

  if (scene_load_group_tmp.job_count == 0) {
    FinalizeSceneModelsTmp();
    scene_load_group_tmp.state = AsyncGroupState::Loaded;
    return;
  }

  StartSceneModelJobs();
}

[[maybe_unused]] static void UnloadSceneNowTmp() {
  COMET_PROFILE("SceneManager::UnloadSceneNowTmp");

  auto* model_handler{entity::EntityFactoryManager::Get().GetModel()};
  auto& entity_manager{entity::EntityManager::Get()};

  if (model_handler != nullptr) {
    if (entity_manager.IsEntity(character_eve_id_tmp)) {
      model_handler->DestroySkeletalImmediate(character_eve_id_tmp);
    }

    if (entity_manager.IsEntity(character_vampire_id_tmp)) {
      model_handler->DestroySkeletalImmediate(character_vampire_id_tmp);
    }

    if (entity_manager.IsEntity(sponza_id_tmp)) {
      model_handler->DestroyStaticImmediate(sponza_id_tmp);
    }

    for (const auto entity_id : stress_ids_tmp) {
      if (entity_manager.IsEntity(entity_id)) {
        model_handler->DestroySkeletalImmediate(entity_id);
      }
    }
  }

  character_eve_id_tmp = entity::kInvalidEntityId;
  character_vampire_id_tmp = entity::kInvalidEntityId;
  sponza_id_tmp = entity::kInvalidEntityId;
  cube_id_tmp = entity::kInvalidEntityId;

  stress_ids_tmp.Clear();
  stress_pos_tmp.Clear();

  are_render_proxy_tmp_spawned = false;
  render_proxy_stress_timer = .0;
  mesh_stress_timer = .0;

  ResetGroup(scene_load_group_tmp);
  ResetGroup(render_proxy_group_tmp);

  internal::scene_load_group_tmp.state = internal::AsyncGroupState::Idle;
  internal::render_proxy_group_tmp.state = internal::AsyncGroupState::Idle;

  internal::are_render_proxy_tmp_spawned = false;
  internal::render_proxy_stress_timer = .0;
  internal::mesh_stress_timer = .0;

  internal::stress_ids_tmp.Clear();
  internal::stress_pos_tmp.Clear();
}

static void FinalizeRenderProxySpawnTmp() {
  COMET_PROFILE("SceneManager::FinalizeRenderProxySpawnTmp");
  auto& entity_manager{entity::EntityManager::Get()};

  for (usize i{0}; i < stress_ids_tmp.GetSize(); ++i) {
    const auto entity_id{stress_ids_tmp[i]};

    if (entity_id == entity::kInvalidEntityId ||
        !entity_manager.IsEntity(entity_id)) {
      continue;
    }

    auto* transform{
        entity_manager.GetComponent<physics::TransformComponent>(entity_id)};

    if (transform == nullptr) {
      continue;
    }

    const auto& pos{stress_pos_tmp[i]};
    physics::TranslateLocal(transform, math::Vec3{pos.x, .0f, pos.y});
  }

  are_render_proxy_tmp_spawned = true;
}

static void StartRenderProxySpawnTmp() {
  COMET_PROFILE("SceneManager::StartRenderProxySpawnTmp");

  if (render_proxy_group_tmp.state == AsyncGroupState::AwaitingModelLoaded ||
      render_proxy_group_tmp.state ==
          AsyncGroupState::WaitingForEntityChanges ||
      render_proxy_group_tmp.state == AsyncGroupState::Finalizing ||
      render_proxy_group_tmp.state == AsyncGroupState::Destroying) {
    return;
  }

  ResetGroup(render_proxy_group_tmp);

  stress_ids_tmp.Clear();
  stress_ids_tmp.Resize(kMaxRenderProxyJobs);

  stress_pos_tmp.Clear();
  stress_pos_tmp.Resize(kMaxRenderProxyJobs);

  usize job_index{0};

  for (usize z{0}; z < kRenderProxyGridZ; ++z) {
    for (usize x{0}; x < kRenderProxyGridX; ++x) {
      stress_ids_tmp[job_index] = entity::kInvalidEntityId;
      stress_pos_tmp[job_index] =
          math::Vec2{static_cast<f32>(x) * kRenderProxySpacing,
                     static_cast<f32>(z) * kRenderProxySpacing};

      auto& params{render_proxy_jobs_tmp[job_index]};
      params.kind = AsyncModelKind::Skeletal;
      params.path = kRenderProxyModelPath;
      params.life_span = resource::ResourceLifeSpan::Manual;
      params.out_entity_id = &stress_ids_tmp[job_index];
      params.completed_count = &render_proxy_group_tmp.completed_count;

      ++job_index;
    }
  }

  render_proxy_group_tmp.job_count = job_index;
  render_proxy_group_tmp.state = AsyncGroupState::AwaitingModelLoaded;

  for (usize i{0}; i < render_proxy_group_tmp.job_count; ++i) {
    KickModelJob(&render_proxy_jobs_tmp[i]);
  }

  entity::ThenAfterEntityChanges(
      AsyncModelGroupFence{&render_proxy_group_tmp}, [] {
        render_proxy_group_tmp.state = AsyncGroupState::Finalizing;
        FinalizeRenderProxySpawnTmp();
        render_proxy_group_tmp.state = AsyncGroupState::Loaded;
      });
}

static void DestroyRenderProxyStressTmp() {
  COMET_PROFILE("SceneManager::DestroyRenderProxyStressTmp");

  if (render_proxy_group_tmp.state == AsyncGroupState::AwaitingModelLoaded ||
      render_proxy_group_tmp.state ==
          AsyncGroupState::WaitingForEntityChanges ||
      render_proxy_group_tmp.state == AsyncGroupState::Finalizing ||
      render_proxy_group_tmp.state == AsyncGroupState::Destroying) {
    return;
  }

  auto& entity_manager{entity::EntityManager::Get()};
  auto* model_handler{entity::EntityFactoryManager::Get().GetModel()};

  COMET_ASSERT(model_handler != nullptr,
               "SceneManager::DestroyRenderProxyStressTmp",
               "model factory handler is null");

  bool has_destroyed_entity{false};

  for (const auto entity_id : stress_ids_tmp) {
    if (entity_id != entity::kInvalidEntityId &&
        entity_manager.IsEntity(entity_id)) {
      model_handler->DestroySkeletal(entity_id);
      has_destroyed_entity = true;
    }
  }

  if (!has_destroyed_entity) {
    stress_ids_tmp.Clear();
    stress_pos_tmp.Clear();
    are_render_proxy_tmp_spawned = false;
    ResetGroup(render_proxy_group_tmp);
    return;
  }

  render_proxy_group_tmp.state = AsyncGroupState::Destroying;

  entity::AfterEntityChanges([] {
    stress_ids_tmp.Clear();
    stress_pos_tmp.Clear();
    are_render_proxy_tmp_spawned = false;
    ResetGroup(render_proxy_group_tmp);
  });
}

static void ToggleRenderProxyStressTmp() {
  if (are_render_proxy_tmp_spawned) {
    DestroyRenderProxyStressTmp();
    return;
  }

  StartRenderProxySpawnTmp();
}

static void MeshStressTmp() {
  auto& entity_manager{entity::EntityManager::Get()};
  auto& frame_manager{frame::FrameManager::Get()};

  entity_manager.ForEach<geometry::MeshComponent, physics::TransformComponent>(
      [&](entity::EntityId entity_id, geometry::MeshComponent& mesh_cmp,
          physics::TransformComponent&) {
        frame_manager.GetLogicFramePacket()->RegisterDirtyMesh(entity_id,
                                                               &mesh_cmp);
      });
}

static void StressComponentAddRemove() {
  auto& entity_manager{entity::EntityManager::Get()};

  constexpr usize kEntityCount{512};
  constexpr usize kIterationCount{2048};

  auto& entities{
      *COMET_FRAME_ARRAY_WITH_CAPACITY(entity::EntityId, kEntityCount)};
  entities.Resize(kEntityCount);

  for (usize i{0}; i < kEntityCount; ++i) {
    const auto id{entity_manager.Generate()};
    entities[i] = id;

    entity_manager.AddComponents(
        id, TestPositionComponent{id, static_cast<f32>(i), .0f, .0f},
        TestHealthComponent{id, 100});
  }

  entity_manager.Flush();

  [[maybe_unused]] const auto baseline_count{entity_manager.GetEntityCount()};

  for (usize iteration{0}; iteration < kIterationCount; ++iteration) {
    for (usize i{0}; i < kEntityCount; ++i) {
      const auto id{entities[i]};

      switch ((iteration + i) % 8) {
        case 0:
          entity_manager.AddComponents(
              id, TestVelocityComponent{id, 1.0f, 2.0f, 3.0f});
          break;

        case 1:
          entity_manager.RemoveComponents<TestVelocityComponent>(id);
          break;

        case 2:
          entity_manager.AddComponents(id, TestMarkerComponent{id});
          break;

        case 3:
          entity_manager.RemoveComponents<TestMarkerComponent>(id);
          break;

        case 4:
          entity_manager.AddComponents(id, TestBigComponent{id});
          break;

        case 5:
          entity_manager.RemoveComponents<TestBigComponent>(id);
          break;

        case 6:
          entity_manager.RemoveComponents<
              TestVelocityComponent, TestMarkerComponent, TestBigComponent>(id);
          break;

        case 7:
          entity_manager.AddComponents(
              id, TestPositionComponent{id, 9.0f, 9.0f, 9.0f},
              TestHealthComponent{id, 42});
          break;
      }
    }

    entity_manager.Flush();

    COMET_ASSERT(entity_manager.GetEntityCount() == baseline_count,
                 "StressComponentAddRemove",
                 "entity count changed during component churn", "baseline",
                 baseline_count, "current", entity_manager.GetEntityCount());
  }

  for (const auto id : entities) {
    entity_manager.Destroy(id);
  }

  entity_manager.Flush();
}
}  // namespace internal

SceneManager& SceneManager::Get() {
  static SceneManager singleton{};
  return singleton;
}

void SceneManager::Update() {
  COMET_PROFILE("SceneManager::Update");

  if (internal::scene_load_group_tmp.state !=
      internal::AsyncGroupState::Loaded) {
    return;
  }

  const auto delta{time::TimeManager::Get().GetUnscaledDeltaTime()};

  if constexpr (internal::kIsRenderProxyStressEnabled) {
    internal::render_proxy_stress_timer += delta;

    if (internal::render_proxy_stress_timer >= 2.0f) {
      internal::render_proxy_stress_timer = .0f;
      internal::ToggleRenderProxyStressTmp();
    }
  }

  if constexpr (internal::kIsMeshStressEnabled) {
    internal::mesh_stress_timer += delta;

    if (internal::mesh_stress_timer >= 2.0f) {
      internal::mesh_stress_timer = .0f;
      internal::MeshStressTmp();
    }
  }

  if constexpr (internal::kIsComponentStressEnabled) {
    internal::StressComponentAddRemove();
  }
}

void SceneManager::LoadScene() {
  COMET_PROFILE("SceneManager::LoadScene");
  internal::LoadSceneTmp();
}

usize SceneManager::GetExpectedEntityCount() const { return 10000; }

void SceneManager::OnInitialize() { RegisterEvents(); }

void SceneManager::OnShutdown() {
  UnregisterEvents();

  internal::scene_load_group_tmp.state = internal::AsyncGroupState::Idle;
  internal::render_proxy_group_tmp.state = internal::AsyncGroupState::Idle;

  internal::are_render_proxy_tmp_spawned = false;
  internal::render_proxy_stress_timer = .0;
  internal::mesh_stress_timer = .0;

  internal::stress_ids_tmp.Clear();
  internal::stress_pos_tmp.Clear();
}

void SceneManager::OnEvent(const event::Event& event) {
  if (event.GetType() == SceneLoadRequestEvent::kStaticType_) {
    LoadScene();
    return;
  }

  if (event.GetType() == entity::ModelLoadedEvent::kStaticType_) {
    const auto& e{static_cast<const entity::ModelLoadedEvent&>(event)};
    const auto id{e.GetEntityId()};

    if (internal::render_proxy_group_tmp.state ==
        internal::AsyncGroupState::AwaitingModelLoaded) {
      for (const auto stress_id : internal::stress_ids_tmp) {
        if (id == stress_id) {
          internal::render_proxy_group_tmp.loaded_count.fetch_add(
              1, std::memory_order_release);
          break;
        }
      }
    }

    if (internal::scene_load_group_tmp.state ==
        internal::AsyncGroupState::AwaitingModelLoaded) {
      if (id == internal::character_eve_id_tmp ||
          id == internal::character_vampire_id_tmp ||
          id == internal::sponza_id_tmp || id == internal::cube_id_tmp) {
        internal::scene_load_group_tmp.loaded_count.fetch_add(
            1, std::memory_order_release);
      }
    }
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

  internal::model_loaded_listener_id = event_manager.Register(
      event_function, entity::ModelLoadedEvent::kStaticType_);

  COMET_ASSERT(
      internal::model_loaded_listener_id != event::kInvalidEventListenerId,
      "SceneManager::RegisterEvents",
      "model loaded listener registration failed");
}

void SceneManager::UnregisterEvents() {
  auto& event_manager{event::EventManager::Get()};

  if (scene_load_request_listener_id_ != event::kInvalidEventListenerId) {
    event_manager.Unregister(scene_load_request_listener_id_);
    scene_load_request_listener_id_ = event::kInvalidEventListenerId;
  }

  if (internal::model_loaded_listener_id != event::kInvalidEventListenerId) {
    event_manager.Unregister(internal::model_loaded_listener_id);
    internal::model_loaded_listener_id = event::kInvalidEventListenerId;
  }
}
}  // namespace scene
}  // namespace comet