// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_engine_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/engine/frame/engine_frame_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/job/job.h"
#include "comet/core/job/job_utils.h"
#include "comet/core/job/scheduler.h"
#include "comet/engine/engine_client.h"
#include "comet/render/render_manager.h"
#include "comet/runtime/animation/animation_manager.h"
#include "comet/runtime/camera/camera_manager.h"
#include "comet/runtime/continuation/continuation_manager.h"
#include "comet/runtime/continuation/continuation_phase.h"
#include "comet/runtime/environment/environment_manager.h"
#include "comet/runtime/event/event_manager.h"
#include "comet/runtime/frame/frame_manager.h"
#include "comet/runtime/light/light_manager.h"
#include "comet/runtime/physics/physics_manager.h"
#include "comet/runtime/profiler/profiler.h"
#include "comet/runtime/scene/scene_manager.h"
#include "comet/runtime/time/time_manager.h"

namespace comet {
EngineFrameManager& EngineFrameManager::Get() {
  static EngineFrameManager singleton{};
  return singleton;
}

void EngineFrameManager::RunFrame(f64& lag, EngineClient& client) {
  COMET_PROFILE("EngineFrameManager::RunFrame");

  auto& frame_manager{frame::FrameManager::Get()};

  BeginFrame(lag, client);

  job::CounterGuard guard{};

  auto* logic_packet{frame_manager.GetLogicFramePacket()};
  auto* render_packet{frame_manager.GetRenderingFramePacket()};

  PrepareFramePackets(logic_packet, render_packet, lag, guard.GetCounter());

  KickLogicUpdate(logic_packet);
  render::RenderManager::Get().Update(render_packet);

  WaitFrameJobs(guard, logic_packet, render_packet, lag);
  EndFrame(frame_manager);
}

void EngineFrameManager::BeginFrame(f64& lag, EngineClient& client) {
  COMET_PROFILE("EngineFrameManager::BeginFrame");

  time::TimeManager::Get().Update();
  lag += time::TimeManager::Get().GetDeltaTime();

  ContinuationManager::Get().Poll(ContinuationPhase::BeginFrame);

  // Client updates camera/world before packet preparation.
  client.OnUpdate(lag);
}

void EngineFrameManager::PrepareFramePackets(frame::FramePacket* logic_packet,
                                             frame::FramePacket* render_packet,
                                             f64 lag, job::Counter* counter) {
  COMET_PROFILE("EngineFrameManager::PrepareFramePackets");

  COMET_ASSERT(logic_packet != nullptr,
               "EngineFrameManager::PrepareFramePackets",
               "logic frame packet is null");
  COMET_ASSERT(render_packet != nullptr,
               "EngineFrameManager::PrepareFramePackets",
               "render frame packet is null");
  COMET_ASSERT(counter != nullptr, "EngineFrameManager::PrepareFramePackets",
               "counter is null");

  logic_packet->lag = lag;
  logic_packet->counter = counter;

  render_packet->counter = counter;

  PrepareLogicPacket(logic_packet);
}

void EngineFrameManager::PrepareLogicPacket(frame::FramePacket* packet) {
  COMET_PROFILE("EngineFrameManager::PrepareLogicPacket");

  COMET_ASSERT(packet != nullptr, "EngineFrameManager::PrepareLogicPacket",
               "frame packet is null");

  const auto extent{render::RenderManager::Get().GetWindowExtent()};

  packet->main_camera_view_index =
      camera::CameraManager::Get().PopulateCameraViews(
          *packet->camera_views, extent.width, extent.height);

#ifdef COMET_DEBUG
  packet->has_debug_camera = packet->camera_views->GetSize() > 1;
#endif  // COMET_DEBUG

  const auto expected_entity_count{
      scene::SceneManager::Get().GetExpectedEntityCount()};

  packet->added_geometries->Reserve(expected_entity_count);
  packet->dirty_meshes->Reserve(expected_entity_count);
  packet->dirty_transforms->Reserve(expected_entity_count);
  packet->removed_geometries->Reserve(expected_entity_count);

  packet->is_populated = true;
}

void EngineFrameManager::KickLogicUpdate(frame::FramePacket* packet) {
  COMET_PROFILE("EngineFrameManager::KickLogicUpdate");

  COMET_ASSERT(packet != nullptr, "EngineFrameManager::KickLogicUpdate",
               "frame packet is null");

  ContinuationManager::Get().Poll(ContinuationPhase::BeforeLogic);
  event::EventManager::Get().FireAllEvents();

  job::Scheduler::Get().Kick(job::GenerateJobDescr(
      job::JobPriority::High,
      [](job::JobParamsHandle params_handle) {
        COMET_PROFILE("EngineFrameManager::LogicUpdateJob");

        auto* packet{reinterpret_cast<frame::FramePacket*>(params_handle)};
        COMET_ASSERT(packet != nullptr, "EngineFrameManager::LogicUpdateJob",
                     "frame packet is null");

        scene::SceneManager::Get().Update();
        light::LightManager::Get().Update(packet);
        environment::EnvironmentManager::Get().Update(packet);
        physics::PhysicsManager::Get().Update(packet);

        packet->interpolation =
            packet->lag / time::TimeManager::Get().GetFixedDeltaTime();

        animation::AnimationManager::Get().Update(packet);

        ContinuationManager::Get().Poll(ContinuationPhase::AfterLogic);
      },
      packet, job::JobStackSize::Large, packet->counter, "logic_update"));
}

void EngineFrameManager::WaitFrameJobs(job::CounterGuard& guard,
                                       frame::FramePacket* logic_packet,
                                       frame::FramePacket* render_packet,
                                       f64& lag) {
  COMET_PROFILE("EngineFrameManager::WaitFrameJobs");

  COMET_ASSERT(logic_packet != nullptr, "EngineFrameManager::WaitFrameJobs",
               "logic frame packet is null");
  COMET_ASSERT(render_packet != nullptr, "EngineFrameManager::WaitFrameJobs",
               "render frame packet is null");

  guard.Wait();

  logic_packet->counter = nullptr;
  render_packet->counter = nullptr;

  lag = logic_packet->lag;
}

void EngineFrameManager::EndFrame(frame::FrameManager& frame_manager) {
  COMET_PROFILE("EngineFrameManager::EndFrame");

  ContinuationManager::Get().Poll(ContinuationPhase::BeforeEndFrame);

  frame_manager.Update();

  ContinuationManager::Get().Poll(ContinuationPhase::AfterEndFrame);
}
}  // namespace comet