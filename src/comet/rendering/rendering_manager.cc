// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "rendering_manager.h"

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/engine.h"
#include "comet/input/input_manager.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/driver/opengl/opengl_driver.h"
#include "comet/rendering/driver/vulkan/vulkan_driver.h"

namespace comet {
namespace rendering {
void RenderingManager::Initialize() {
  Manager::Initialize();
  const auto fps_cap{COMET_CONF_U16(conf::kRenderingFpsCap)};

  if (fps_cap > 0) {
    frame_time_threshold_ =
        1000 / static_cast<f64>(COMET_CONF_U16(conf::kRenderingFpsCap));
  } else {
    frame_time_threshold_ = 0;
  }

  const auto driver_type{
      GetDriverTypeFromStr(COMET_CONF_STR(conf::kRenderingDriver))};

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
  Engine::Get().GetInputManager().AttachGlfwWindow(
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
  current_time_ += Engine::Get().GetTimeManager().GetDeltaTime();

  if (current_time_ > 1000) {
    frame_rate_ = counter_;
    current_time_ = 0;
    counter_ = 0;
  }

  if (IsFpsCapReached()) {
    return;
  }

  driver_->Update(interpolation);
  Engine::Get().GetInputManager().Update();
  ++counter_;
}

const Window* RenderingManager::GetWindow() const {
  if (driver_ == nullptr) {
    return nullptr;
  }

  return driver_->GetWindow();
}

u32 RenderingManager::GetFrameRate() const noexcept { return frame_rate_; }

f32 RenderingManager::GetFrameTime() const noexcept {
  return (1 / static_cast<f32>(frame_rate_)) * 1000;
}

void RenderingManager::GenerateOpenGlDriver() {
  COMET_LOG_RENDERING_DEBUG("Initializing OpenGL driver...");
  COMET_LOG_RENDERING_WARNING("OpenGL implementation is not ready yet.");

  gl::OpenGlDriverDescr descr{};
  FillDriverDescr(descr);
  descr.opengl_major_version =
      COMET_CONF_U8(conf::kRenderingOpenGlMajorVersion);
  descr.opengl_minor_version =
      COMET_CONF_U8(conf::kRenderingOpenGlMinorVersion);

  driver_ = std::make_unique<gl::OpenGlDriver>(descr);
}

void RenderingManager::GenerateVulkanDriver() {
  COMET_LOG_RENDERING_DEBUG("Initializing Vulkan driver...");

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
  COMET_LOG_RENDERING_DEBUG("Initializing Direct3D 12 driver...");
  COMET_ASSERT(false, "Direct3D 12 is unsupported at this time.");
}

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

  descr.app_name = COMET_CONF_STR(conf::kApplicationName);
  descr.app_major_version = COMET_CONF_U8(conf::kRenderingVulkanMajorVersion);
  descr.app_minor_version = COMET_CONF_U8(conf::kRenderingVulkanMinorVersion);
  descr.app_patch_version = COMET_CONF_U8(conf::kRenderingVulkanPatchVersion);
}

std::vector<RenderingViewDescr> RenderingManager::GenerateRenderingViewDescrs()
    const {
  std::vector<RenderingViewDescr> descrs;

  // TODO(m4jr0): Implement the rest of the views.
#ifndef COMET_IMGUI
  descrs.resize(1);
#else
  descrs.resize(2);
#endif  // !COMET_IMGUI

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

  constexpr auto kWorldViewIndex{0};
  constexpr auto kImGuiViewIndex{1};

  // TODO(m4jr0): Do this from configuration.
  auto& world_view_descr{descrs[kWorldViewIndex]};
  world_view_descr.matrix_source = RenderingViewMatrixSource::SceneCamera;
  world_view_descr.type = RenderingViewType::World;
  world_view_descr.is_first = kWorldViewIndex == 0;
  world_view_descr.is_last = kWorldViewIndex == descrs.size() - 1;
  world_view_descr.width = window_width;
  world_view_descr.height = window_height;
  std::memcpy(world_view_descr.clear_color, clear_color, sizeof(clear_color));
  world_view_descr.id = COMET_STRING_ID("rendering_world_view");

  //  // TODO(m4jr0): Implement these views.
  //  auto& skybox_view_descr{descrs[1]};
  //  world_view_descr.matrix_source = RenderingViewMatrixSource::SceneCamera;
  //  world_view_descr.type = RenderingViewType::Skybox;
  //  world_view_descr.width = window_width;
  //  world_view_descr.height = window_height;
  //  std::memcpy(world_view_descr.clear_color, clear_color,
  //  sizeof(clear_color)); world_view_descr.id =
  //  COMET_STRING_ID("rendering_skybox_view");
  //
  //  auto& light_world_view_descr{descrs[2]};
  //  world_view_descr.matrix_source = RenderingViewMatrixSource::SceneCamera;
  //  world_view_descr.type = RenderingViewType::SimpleWorld;
  //  world_view_descr.width = window_width;
  //  world_view_descr.height = window_height;
  //  std::memcpy(world_view_descr.clear_color, clear_color,
  //  sizeof(clear_color)); world_view_descr.id =
  //  COMET_STRING_ID("rendering_light_world_view");
  //
#ifdef COMET_IMGUI
  auto& imgui_view_descr{descrs[kImGuiViewIndex]};
  imgui_view_descr.matrix_source = RenderingViewMatrixSource::Unknown;
  imgui_view_descr.type = RenderingViewType::ImGui;
  imgui_view_descr.is_first = kImGuiViewIndex == 0;
  imgui_view_descr.is_last = kImGuiViewIndex == descrs.size() - 1;
  imgui_view_descr.width = window_width;
  imgui_view_descr.height = window_height;
  std::memcpy(imgui_view_descr.clear_color, clear_color, sizeof(clear_color));
  imgui_view_descr.id = COMET_STRING_ID("rendering_imgui_view");
#endif  // COMET_IMGUI

  return descrs;
}

bool RenderingManager::IsFpsCapReached() const {
  return frame_time_threshold_ > 0 &&
         current_time_ / frame_time_threshold_ < counter_;
}
}  // namespace rendering
}  // namespace comet
