// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "rendering_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/input/input_manager.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/driver/empty/empty_driver.h"
#include "comet/rendering/driver/opengl/opengl_driver.h"
#include "comet/rendering/driver/vulkan/vulkan_driver.h"
#include "comet/rendering/utils/driver_utils.h"
#include "comet/time/time_manager.h"

#ifdef COMET_HAS_DEBUG_UI
#include "comet/rendering/debug_ui_registry.h"
#endif  // COMET_HAS_DEBUG_UI

namespace comet {
namespace rendering {
RenderingManager& RenderingManager::Get() {
  static RenderingManager singleton{};
  return singleton;
}

void RenderingManager::Update(frame::FramePacket* packet) {
  COMET_PROFILE("RenderingManager::Update");
  COMET_ASSERT(packet != nullptr, "RenderingManager::Update",
               "frame packet is null");

  if (!packet->is_populated) {
    return;
  }

  current_time_ += time::TimeManager::Get().GetUnscaledDeltaTime();

  if (current_time_ > 1.0) {
    frame_rate_ = counter_;
    current_time_ = .0;
    counter_ = 0;
  }

  packet->can_present = !IsFpsCapReached();

  struct Job {
    frame::FramePacket* packet{nullptr};
    RenderProxyRecordStore* render_proxy_record_store{nullptr};
    Driver* driver{nullptr};
  };

  auto* job{COMET_DOUBLE_FRAME_ALLOC_ONE_AND_POPULATE(Job)};
  job->packet = packet;
  job->render_proxy_record_store = &render_proxy_record_store_;
  job->driver = driver_.get();

  if (is_multithreaded_) {
    auto& scheduler{job::Scheduler::Get()};

    scheduler.Kick(job::GenerateJobDescr(
        job::JobPriority::High,
        [](job::JobParamsHandle params_handle) {
          auto* job{reinterpret_cast<Job*>(params_handle)};
          job->render_proxy_record_store->Update(job->packet);
          job->driver->Update(job->packet);
        },
        job, job::JobStackSize::Large, packet->counter,
        "rendering_driver_update"));

    input::InputManager::Get().Update();
  } else {
    job::Scheduler::Get().KickOnMainThread(job::GenerateMainThreadJobDescr(
        [](job::MainThreadParamsHandle params_handle) {
          auto* job{reinterpret_cast<Job*>(params_handle)};
          job->render_proxy_record_store->Update(job->packet);
          job->driver->Update(job->packet);

          input::InputManager::Get().Update();
        },
        job, packet->counter));
  }

  ++counter_;
}

WindowExtent RenderingManager::GetWindowExtent() const {
  COMET_ASSERT(driver_ != nullptr, "RenderingManager::OnShutdown",
               "rendering driver is null");
  const auto* window{driver_->GetWindow()};

  return {
      .width = window->GetWidth(),
      .height = window->GetHeight(),
  };
}

DriverType RenderingManager::GetDriverType() const noexcept {
  COMET_ASSERT(driver_ != nullptr, "RenderingManager::OnShutdown",
               "rendering driver is null");
  return driver_->GetType();
}

u32 RenderingManager::GetFrameRate() const noexcept { return frame_rate_; }

f64 RenderingManager::GetFrameTime() const noexcept {
  return frame_rate_ == 0 ? .0 : (1.0 / static_cast<f64>(frame_rate_));
}

u32 RenderingManager::GetDrawCount() const noexcept {
  COMET_ASSERT(driver_ != nullptr, "RenderingManager::OnShutdown",
               "rendering driver is null");
  return driver_->GetDrawCount();
}

const ShadowSettings& RenderingManager::GetShadowSettings() const noexcept {
  return shadow_settings_;
}

bool RenderingManager::IsMultithreaded() const noexcept {
  return is_multithreaded_;
}

void RenderingManager::OnInitialize() {
  const auto fps_cap{COMET_CONF_U16(conf::kRenderingFpsCap)};

  if (fps_cap > 0) {
    frame_time_threshold_ =
        1.0f / static_cast<f64>(COMET_CONF_U16(conf::kRenderingFpsCap));
  } else {
    frame_time_threshold_ = 0;
  }

  shadow_settings_ = GenerateShadowSettings();
  render_proxy_record_store_.Initialize(kDefaultRenderProxyCount_);

  const auto* driver_label{COMET_CONF_STR(conf::kRenderingDriver)};
  COMET_LOG_INFO(LoggerType::Rendering, "RenderingManager::OnInitialize",
                 "graphics backend selected", "driver_label", driver_label);

  const auto driver_type{GetDriverTypeFromStr(driver_label)};
  COMET_ASSERT(
      driver_type != DriverType::Unknown, "RenderingManager::OnInitialize",
      "rendering driver type is unknown", "driver_label", driver_label);

  if (driver_type == DriverType::OpenGl) {
    GenerateOpenGlDriver();
  } else if (driver_type == DriverType::Vulkan) {
    GenerateVulkanDriver();
  } else if (driver_type == DriverType::Direct3d12) {
    GenerateDirect3D12Driver();
  }
#ifdef COMET_DEBUG
  else if (driver_type == DriverType::Empty) {
    GenerateEmptyDriver();
  }
#endif  // COMET_DEBUG

  COMET_ASSERT(driver_ != nullptr, "RenderingManager::OnInitialize",
               "rendering driver is null");
  is_multithreaded_ = IsMultithreading(driver_type);

  // Driver can't be initialized in another thread, as it would not be
  // thread-safe with GLFW.
  if (is_multithreaded_) {
    driver_->Initialize();
  } else {
#ifdef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
    job::CounterGuard guard{};

    job::Scheduler::Get().KickOnMainThread(job::GenerateMainThreadJobDescr(
        [](job::MainThreadParamsHandle params_handle) {
          auto* rendering_manager{
              reinterpret_cast<RenderingManager*>(params_handle)};
          rendering_manager->driver_->Initialize();
        },
        this, guard.GetCounter()));

    guard.Wait();
#else
    COMET_ASSERT(false, "RenderingManager::OnInitialize",
                 "main thread worker is unavailable");
#endif  //  COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  }

  input::InputManager::Get().AttachGlfwWindow(
      static_cast<GlfwWindow*>(driver_->GetWindow())->GetHandle());

#ifdef COMET_IMGUI
  if (driver_type != DriverType::Empty) {
    input::InputManager::EnableImGui();
  }
#endif  // COMET_IMGUI

#ifdef COMET_HAS_DEBUG_UI
  DebugUiRegistry::Get().Initialize();
#endif  // COMET_HAS_DEBUG_UI
}

void RenderingManager::OnShutdown() {
#ifdef COMET_HAS_DEBUG_UI
  DebugUiRegistry::Get().Destroy();
#endif  // COMET_HAS_DEBUG_UI

  if (driver_ != nullptr) {
    driver_->Shutdown();
    driver_ = nullptr;
  }

  render_proxy_record_store_.Shutdown();

  frame_rate_ = 0;
  counter_ = 0;
  frame_time_threshold_ = 0;
  current_time_ = 0;
}

void RenderingManager::GenerateOpenGlDriver() {
  gl::OpenGlDriverDescr descr{};
  FillDriverDescr(descr);
  descr.opengl_major_version =
      COMET_CONF_U8(conf::kRenderingOpenGlMajorVersion);
  descr.opengl_minor_version =
      COMET_CONF_U8(conf::kRenderingOpenGlMinorVersion);

  driver_ = std::make_unique<gl::OpenGlDriver>(descr);
}

void RenderingManager::GenerateVulkanDriver() {
  vk::VulkanDriverDescr descr{};
  FillDriverDescr(descr);
  descr.vulkan_major_version =
      COMET_CONF_U8(conf::kRenderingVulkanMajorVersion);
  descr.vulkan_minor_version =
      COMET_CONF_U8(conf::kRenderingVulkanMinorVersion);
  descr.vulkan_patch_version =
      COMET_CONF_U8(conf::kRenderingVulkanPatchVersion);
  descr.vulkan_variant_version =
      COMET_CONF_U8(conf::kRenderingVulkanVariantVersion);
  descr.max_frames_in_flight =
      COMET_CONF_U8(conf::kRenderingVulkanMaxFramesInFlight);

  driver_ = std::make_unique<vk::VulkanDriver>(descr);
}

void RenderingManager::GenerateDirect3D12Driver() {
  COMET_ASSERT(false, "RenderingManager::GenerateDirect3D12Driver",
               "direct3d12 is unsupported");
}

#ifdef COMET_DEBUG
void RenderingManager::GenerateEmptyDriver() {
  empty::EmptyDriverDescr descr{};
  FillDriverDescr(descr);
  driver_ = std::make_unique<empty::EmptyDriver>(descr);
}
#endif  // COMET_DEBUG

void RenderingManager::FillDriverDescr(DriverDescr& descr) const {
  descr.is_vsync = COMET_CONF_BOOL(conf::kRenderingIsVsync);
  descr.is_triple_buffering =
      COMET_CONF_BOOL(conf::kRenderingIsTripleBuffering);

  descr.clear_color[0] = COMET_CONF_F32(conf::kRenderingClearColorR);
  descr.clear_color[1] = COMET_CONF_F32(conf::kRenderingClearColorG);
  descr.clear_color[2] = COMET_CONF_F32(conf::kRenderingClearColorB);
  descr.clear_color[3] = COMET_CONF_F32(conf::kRenderingClearColorA);

  descr.window_width =
      static_cast<WindowSize>(COMET_CONF_U16(conf::kRenderingWindowWidth));
  descr.window_height =
      static_cast<WindowSize>(COMET_CONF_U16(conf::kRenderingWindowHeight));

  descr.anti_aliasing_type =
      GetAntiAliasingTypeFromStr(COMET_CONF_STR(conf::kRenderingAntiAliasing));

  descr.is_sampler_anisotropy =
      COMET_CONF_BOOL(conf::kRenderingIsSamplerAnisotropy);
  descr.is_sample_rate_shading =
      COMET_CONF_BOOL(conf::kRenderingIsSampleRateShading);

  descr.rendering_view_descrs = GenerateRenderingViewDescrs();

  const auto* app_name{COMET_CONF_STR(conf::kApplicationName)};

  descr.app_name_len = GetLength(app_name);
  COMET_ASSERT(descr.app_name_len < kMaxAppNameLen,
               "RenderingManager::FillDriverDescr",
               "application name is too long", "app_name_len",
               descr.app_name_len, "max_app_name_len", kMaxAppNameLen);

  Copy(descr.app_name, app_name, descr.app_name_len);
  descr.app_major_version = COMET_CONF_U8(conf::kRenderingVulkanMajorVersion);
  descr.app_minor_version = COMET_CONF_U8(conf::kRenderingVulkanMinorVersion);
  descr.app_patch_version = COMET_CONF_U8(conf::kRenderingVulkanPatchVersion);

  descr.shadow_settings = &shadow_settings_;
  descr.render_proxy_record_store = &render_proxy_record_store_;
}

frame::FrameArray<RenderingViewDescr>
RenderingManager::GenerateRenderingViewDescrs() const {
  frame::FrameArray<RenderingViewDescr> descrs{};

  usize size{2};

#ifdef COMET_DEBUG_VIEW
  ++size;
#endif  // COMET_DEBUG_VIEW

#ifdef COMET_IMGUI
  ++size;
#endif  // COMET_IMGUI

  descrs.Resize(size);

  f32 clear_color[4]{};
  clear_color[0] = COMET_CONF_F32(conf::kRenderingClearColorR);
  clear_color[1] = COMET_CONF_F32(conf::kRenderingClearColorG);
  clear_color[2] = COMET_CONF_F32(conf::kRenderingClearColorB);
  clear_color[3] = COMET_CONF_F32(conf::kRenderingClearColorA);

  const auto window_width{
      static_cast<WindowSize>(COMET_CONF_U16(conf::kRenderingWindowWidth)),
  };

  const auto window_height{
      static_cast<WindowSize>(COMET_CONF_U16(conf::kRenderingWindowHeight)),
  };

  usize cursor{0};

  auto& shadow_view_descr{descrs[cursor]};
  shadow_view_descr.type = RenderingViewType::Shadow;
  shadow_view_descr.width =
      static_cast<WindowSize>(shadow_settings_.resolution);
  shadow_view_descr.height =
      static_cast<WindowSize>(shadow_settings_.resolution);
  shadow_view_descr.id = COMET_STRING_ID("rendering_shadow_view");
  ++cursor;

  // TODO(m4jr0): Do this from configuration.
  auto& world_view_descr{descrs[cursor]};
  world_view_descr.matrix_source = RenderingViewMatrixSource::SceneCamera;
  world_view_descr.type = RenderingViewType::World;
  world_view_descr.width = window_width;
  world_view_descr.height = window_height;
  memory::CopyMemory(world_view_descr.clear_color, clear_color,
                     sizeof(clear_color));
  world_view_descr.id = COMET_STRING_ID("rendering_world_view");
  ++cursor;

#ifdef COMET_DEBUG_VIEW
  auto& debug_view_descr{descrs[cursor]};
  debug_view_descr.matrix_source = RenderingViewMatrixSource::SceneCamera;
  debug_view_descr.type = RenderingViewType::Debug;
  debug_view_descr.width = window_width;
  debug_view_descr.height = window_height;
  memory::CopyMemory(debug_view_descr.clear_color, clear_color,
                     sizeof(clear_color));
  debug_view_descr.id = COMET_STRING_ID("rendering_debug_view");
  ++cursor;
#endif  // COMET_DEBUG_VIEW

#ifdef COMET_IMGUI
  auto& imgui_view_descr{descrs[cursor]};
  imgui_view_descr.matrix_source = RenderingViewMatrixSource::Unknown;
  imgui_view_descr.type = RenderingViewType::ImGui;
  imgui_view_descr.width = window_width;
  imgui_view_descr.height = window_height;
  memory::CopyMemory(imgui_view_descr.clear_color, clear_color,
                     sizeof(clear_color));
  imgui_view_descr.id = COMET_STRING_ID("rendering_imgui_view");
  ++cursor;
#endif  // COMET_IMGUI

  return descrs;
}

ShadowSettings RenderingManager::GenerateShadowSettings() const {
  ShadowSettings settings{};
  settings.resolution =
      static_cast<u32>(COMET_CONF_U16(conf::kRenderingShadowResolution));
  settings.max_distance = COMET_CONF_F32(conf::kRenderingShadowDistance);
  settings.cascade_count =
      static_cast<u32>(COMET_CONF_U8(conf::kRenderingShadowCascadeCount));
  settings.cascade_lambda = COMET_CONF_F32(conf::kRenderingShadowCascadeLambda);
  settings.bias_constant = COMET_CONF_F32(conf::kRenderingShadowBiasConstant);
  settings.bias_slope = COMET_CONF_F32(conf::kRenderingShadowBiasSlope);

  settings.caster_extrusion_factor =
      COMET_CONF_F32(conf::kRenderingShadowCasterExtrusionFactor);
  settings.receiver_pad_xy =
      COMET_CONF_F32(conf::kRenderingShadowReceiverPadXY);
  settings.receiver_pad_z = COMET_CONF_F32(conf::kRenderingShadowReceiverPadZ);

  settings.cascade_blend_ratio =
      COMET_CONF_F32(conf::kRenderingShadowCascadeBlendRatio);

  settings.pcf_radius = COMET_CONF_F32(conf::kRenderingShadowPcfRadius);
  settings.pcf_samples =
      static_cast<u32>(COMET_CONF_U8(conf::kRenderingShadowPcfSamples));

  settings.is_debug_cascades =
      COMET_CONF_BOOL(conf::kRenderingShadowDebugCascades);
  settings.debug_single_cascade =
      static_cast<s32>(COMET_CONF_S8(conf::kRenderingShadowDebugSingleCascade));
  settings.is_blending_disabled =
      COMET_CONF_BOOL(conf::kRenderingShadowDisableBlending);

  return settings;
}

bool RenderingManager::IsFpsCapReached() const {
  return frame_time_threshold_ > 0 &&
         current_time_ / frame_time_threshold_ < counter_;
}
}  // namespace rendering
}  // namespace comet
