// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "rendering_manager.h"

#include "comet/rendering/driver/driver.h"
#include "comet/rendering/driver/opengl/opengl_driver.h"
#include "comet/rendering/driver/vulkan/vulkan_driver.h"

namespace comet {
namespace rendering {
RenderingManager::RenderingManager(const RenderingManagerDescr& descr)
    : Manager{descr},
      camera_manager_{descr.camera_manager},
      configuration_manager_{descr.configuration_manager},
#ifdef COMET_DEBUG
      debugger_displayer_manager_{descr.debugger_displayer_manager},
#endif  // COMET_DEBUG
      entity_manager_{descr.entity_manager},
      event_manager_{descr.event_manager},
      input_manager_{descr.input_manager},
      resource_manager_{descr.resource_manager},
      time_manager_{descr.time_manager} {
  COMET_ASSERT(camera_manager_ != nullptr, "Camera manager is null!");
  COMET_ASSERT(configuration_manager_ != nullptr,
               "Configuration manager is null!");
#ifdef COMET_DEBUG
  COMET_ASSERT(debugger_displayer_manager_ != nullptr,
               "Debugger displayer manager is null!");
#endif  // COMET_DEBUG
  COMET_ASSERT(entity_manager_ != nullptr, "Entity manager is null!");
  COMET_ASSERT(event_manager_ != nullptr, "Event manager is null!");
  COMET_ASSERT(input_manager_ != nullptr, "Input manager is null!");
  COMET_ASSERT(resource_manager_ != nullptr, "Resource manager is null!");
  COMET_ASSERT(time_manager_ != nullptr, "Time manager is null!");
}

void RenderingManager::Initialize() {
  Manager::Initialize();
  const auto fps_cap{configuration_manager_->GetU16(conf::kRenderingFpsCap)};

  if (fps_cap > 0) {
    frame_time_threshold_ =
        1000 / static_cast<f64>(
                   configuration_manager_->GetU16(conf::kRenderingFpsCap));
  } else {
    frame_time_threshold_ = 0;
  }

  const auto driver_type{GetDriverTypeFromStr(
      configuration_manager_->GetStr(conf::kRenderingDriver))};

  COMET_ASSERT(driver_type != DriverType::Unknown,
               "Unknown rendering driver type!");

  if (driver_type == DriverType::OpenGl) {
    GenerateOpenGlDriver();
  } else if (driver_type == DriverType::Vulkan) {
    GenerateVulkanDriver();
  } else if (driver_type == DriverType::Direct3d12) {
    GenerateDirect3D12Driver();
  }

  COMET_ASSERT(driver_ != nullptr, "Rendering driver is null!");
  driver_->Initialize();
  input_manager_->AttachGlfwWindow(
      static_cast<GlfwWindow*>(driver_->GetWindow())->GetHandle());
}

void RenderingManager::Shutdown() {
  driver_->Shutdown();
  driver_ = nullptr;
  frame_rate_ = 0;
  counter_ = 0;
  frame_time_threshold_ = 0;
  current_time_ = 0;
  Manager::Shutdown();
}

void RenderingManager::Update(time::Interpolation interpolation) {
  current_time_ += time_manager_->GetDeltaTime();

  if (current_time_ > 1000) {
    frame_rate_ = counter_;
    current_time_ = 0;
    counter_ = 0;
  }

  if (IsFpsCapReached()) {
    return;
  }

  driver_->Update(interpolation);
  input_manager_->Update();
  ++counter_;
}

const Window* RenderingManager::GetWindow() const {
  if (driver_ == nullptr) {
    return nullptr;
  }

  return driver_->GetWindow();
}

rendering::DriverType RenderingManager::GetDriverType() const noexcept {
  return driver_->GetType();
}

u32 RenderingManager::GetFrameRate() const noexcept { return frame_rate_; }

f32 RenderingManager::GetFrameTime() const noexcept {
  return (1 / static_cast<f32>(frame_rate_)) * 1000;
}

uindex RenderingManager::GetDrawCount() const {
  return driver_->GetDrawCount();
}

void RenderingManager::GenerateOpenGlDriver() {
  COMET_LOG_RENDERING_DEBUG("Initializing OpenGL driver...");

  gl::OpenGlDriverDescr descr{};
  FillDriverDescr(descr);
  descr.opengl_major_version =
      configuration_manager_->GetU8(conf::kRenderingOpenGlMajorVersion);
  descr.opengl_minor_version =
      configuration_manager_->GetU8(conf::kRenderingOpenGlMinorVersion);

  driver_ = std::make_unique<gl::OpenGlDriver>(descr);
}

void RenderingManager::GenerateVulkanDriver() {
  COMET_LOG_RENDERING_DEBUG("Initializing Vulkan driver...");

  vk::VulkanDriverDescr descr{};
  FillDriverDescr(descr);
  descr.vulkan_major_version =
      configuration_manager_->GetU8(conf::kRenderingVulkanMajorVersion);
  descr.vulkan_minor_version =
      configuration_manager_->GetU8(conf::kRenderingVulkanMinorVersion);
  descr.vulkan_patch_version =
      configuration_manager_->GetU8(conf::kRenderingVulkanPatchVersion);
  descr.vulkan_variant_version =
      configuration_manager_->GetU8(conf::kRenderingVulkanVariantVersion);
  descr.max_frames_in_flight =
      configuration_manager_->GetU8(conf::kRenderingVulkanMaxFramesInFlight);

  driver_ = std::make_unique<vk::VulkanDriver>(descr);
}

void RenderingManager::GenerateDirect3D12Driver() {
  COMET_LOG_RENDERING_DEBUG("Initializing Direct3D 12 driver...");
  COMET_ASSERT(false, "Direct3D 12 is unsupported at this time.");
}

void RenderingManager::FillDriverDescr(DriverDescr& descr) const {
  descr.is_vsync = configuration_manager_->GetBool(conf::kRenderingIsVsync);
  descr.is_triple_buffering =
      configuration_manager_->GetBool(conf::kRenderingIsTripleBuffering);

  descr.clear_color[0] =
      configuration_manager_->GetF32(conf::kRenderingClearColorR);
  descr.clear_color[1] =
      configuration_manager_->GetF32(conf::kRenderingClearColorG);
  descr.clear_color[2] =
      configuration_manager_->GetF32(conf::kRenderingClearColorB);
  descr.clear_color[3] =
      configuration_manager_->GetF32(conf::kRenderingClearColorA);

  descr.window_width = static_cast<WindowSize>(
      configuration_manager_->GetU16(conf::kRenderingWindowWidth));
  descr.window_height = static_cast<WindowSize>(
      configuration_manager_->GetU16(conf::kRenderingWindowHeight));

  descr.anti_aliasing_type = GetAntiAliasingTypeFromStr(
      configuration_manager_->GetStr(conf::kRenderingAntiAliasing));
  descr.is_sampler_anisotropy =
      configuration_manager_->GetBool(conf::kRenderingIsSamplerAnisotropy);
  descr.is_sample_rate_shading =
      configuration_manager_->GetBool(conf::kRenderingIsSampleRateShading);

  descr.rendering_view_descrs = GenerateRenderingViewDescrs();

  descr.app_name = configuration_manager_->GetStr(conf::kApplicationName);
  descr.app_major_version =
      configuration_manager_->GetU8(conf::kRenderingVulkanMajorVersion);
  descr.app_minor_version =
      configuration_manager_->GetU8(conf::kRenderingVulkanMinorVersion);
  descr.app_patch_version =
      configuration_manager_->GetU8(conf::kRenderingVulkanPatchVersion);
  descr.camera_manager = camera_manager_;
  descr.configuration_manager = configuration_manager_;
#ifdef COMET_DEBUG
  descr.debugger_displayer_manager = debugger_displayer_manager_;
#endif  // COMET_DEBUG
  descr.entity_manager = entity_manager_;
  descr.event_manager = event_manager_;
  descr.resource_manager = resource_manager_;
}

std::vector<RenderingViewDescr> RenderingManager::GenerateRenderingViewDescrs()
    const {
  std::vector<RenderingViewDescr> descrs{};

  uindex size{1};

#ifdef COMET_DEBUG_VIEW
  ++size;
#endif  // COMET_DEBUG_VIEW

  // TODO(m4jr0): Implement the rest of the views.
#ifdef COMET_IMGUI
  ++size;
#endif  // COMET_IMGUI

  descrs.resize(size);

  f32 clear_color[4]{};
  clear_color[0] = configuration_manager_->GetF32(conf::kRenderingClearColorR);
  clear_color[1] = configuration_manager_->GetF32(conf::kRenderingClearColorG);
  clear_color[2] = configuration_manager_->GetF32(conf::kRenderingClearColorB);
  clear_color[3] = configuration_manager_->GetF32(conf::kRenderingClearColorA);

  const auto window_width{
      static_cast<WindowSize>(
          configuration_manager_->GetU16(conf::kRenderingWindowWidth)),
  };

  const auto window_height{
      static_cast<WindowSize>(
          configuration_manager_->GetU16(conf::kRenderingWindowHeight)),
  };

  uindex cursor{0};

  // TODO(m4jr0): Do this from configuration.
  auto& world_view_descr{descrs[cursor]};
  world_view_descr.matrix_source = RenderingViewMatrixSource::SceneCamera;
  world_view_descr.type = RenderingViewType::World;
  world_view_descr.is_first = cursor == 0;
  world_view_descr.is_last = cursor == descrs.size() - 1;
  world_view_descr.width = window_width;
  world_view_descr.height = window_height;
  std::memcpy(world_view_descr.clear_color, clear_color, sizeof(clear_color));
  world_view_descr.id = COMET_STRING_ID("rendering_world_view");
  ++cursor;

  // TODO(m4jr0): Implement these views.
  // auto& skybox_view_descr{descrs[cursor]};
  // skybox_view_descr.matrix_source = RenderingViewMatrixSource::SceneCamera;
  // skybox_view_descr.type = RenderingViewType::Skybox;
  // skybox_view_descr.is_first = cursor == 0;
  // skybox_view_descr.is_last = cursor == descrs.size() - 1;
  // skybox_view_descr.width = window_width;
  // skybox_view_descr.height = window_height;
  // std::memcpy(skybox_view_descr.clear_color, clear_color,
  // sizeof(clear_color)); skybox_view_descr.id =
  // COMET_STRING_ID("rendering_skybox_view");
  // ++cursor;

  // auto& light_world_view_descr{descrs[cursor]};
  // light_world_view_descr.matrix_source =
  // RenderingViewMatrixSource::SceneCamera; light_world_view_descr.type =
  // RenderingViewType::SimpleWorld;
  // light_world_view_descr.is_first = cursor == 0;
  // light_world_view_descr.is_last = cursor == descrs.size() - 1;
  // light_world_view_descr.width = window_width;
  // light_world_view_descr.height = window_height;
  // std::memcpy(light_world_view_descr.clear_color, clear_color,
  //             sizeof(clear_color));
  // light_world_view_descr.id = COMET_STRING_ID("rendering_light_world_view");
  // ++cursor;

#ifdef COMET_DEBUG_VIEW
  auto& debug_view_descr{descrs[cursor]};
  debug_view_descr.matrix_source = RenderingViewMatrixSource::SceneCamera;
  debug_view_descr.type = RenderingViewType::Debug;
  debug_view_descr.is_first = cursor == 0;
  debug_view_descr.is_last = cursor == descrs.size() - 1;
  debug_view_descr.width = window_width;
  debug_view_descr.height = window_height;
  std::memcpy(debug_view_descr.clear_color, clear_color, sizeof(clear_color));
  debug_view_descr.id = COMET_STRING_ID("rendering_debug_view");
  ++cursor;
#endif  // COMET_DEBUG_VIEW

#ifdef COMET_IMGUI
  auto& imgui_view_descr{descrs[cursor]};
  imgui_view_descr.matrix_source = RenderingViewMatrixSource::Unknown;
  imgui_view_descr.type = RenderingViewType::ImGui;
  imgui_view_descr.is_first = cursor == 0;
  imgui_view_descr.is_last = cursor == descrs.size() - 1;
  imgui_view_descr.width = window_width;
  imgui_view_descr.height = window_height;
  std::memcpy(imgui_view_descr.clear_color, clear_color, sizeof(clear_color));
  imgui_view_descr.id = COMET_STRING_ID("rendering_imgui_view");
  ++cursor;
#endif  // COMET_IMGUI

  return descrs;
}

bool RenderingManager::IsFpsCapReached() const {
  return frame_time_threshold_ > 0 &&
         current_time_ / frame_time_threshold_ < counter_;
}
}  // namespace rendering
}  // namespace comet
