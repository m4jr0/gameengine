// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "rendering_manager.h"

#include "comet/core/configuration_manager.h"
#include "comet/core/engine.h"
#include "comet/rendering/driver/opengl/opengl_driver.h"
#include "comet/rendering/driver/vulkan/vulkan_driver.h"

namespace comet {
namespace rendering {
void RenderingManager::Initialize() {
  const auto rendering_driver{
      Engine::Get().GetConfigurationManager().Get<std::string>(
          "rendering_driver")};

  if (rendering_driver == "opengl") {
    GenerateOpenGlDriver();
  } else if (rendering_driver == "vulkan") {
    GenerateVulkanDriver();
  } else if (rendering_driver == "direct3d12") {
    GenerateDirect3D12Driver();
  } else {
    // TODO(m4jr0): Use proper exception.
    throw std::runtime_error("Unknown rendering driver requested: " +
                             rendering_driver);
  }

  driver_->Initialize();
}

void RenderingManager::Destroy() { driver_->Destroy(); }

void RenderingManager::Update(time::Interpolation interpolation,
                              entity::EntityManager& entity_manager) {
  current_time_ += Engine::Get().GetTimeManager().GetTimeDelta();

  if (current_time_ > 1000) {
    current_time_ = 0;
    counter_ = 0;
  }

  driver_->Update(interpolation, entity_manager);
  Engine::Get().GetInputManager().Update();
  ++counter_;
}

const Window* RenderingManager::GetWindow() const {
  if (driver_ == nullptr) {
    return nullptr;
  }

  return &driver_->GetWindow();
}

void RenderingManager::GenerateOpenGlDriver() {
  COMET_LOG_RENDERING_DEBUG("Initializing OpenGL driver...");
  COMET_LOG_RENDERING_WARNING("OpenGL implementation is not ready yet.");

  const auto& conf{Engine::Get().GetConfigurationManager()};

  gl::OpenGlDriverDescr descr{};
  descr.name = conf.Get<std::string>("application_name");
  descr.width = conf.Get<u16>("rendering_window_width");
  descr.height = conf.Get<u16>("rendering_window_height");
  descr.major_version = conf.Get<u8>("rendering_opengl_major_version");
  descr.minor_version = conf.Get<u8>("rendering_opengl_minor_version");

  driver_ = std::make_unique<gl::OpenGlDriver>(descr);
}

void RenderingManager::GenerateVulkanDriver() {
  COMET_LOG_RENDERING_DEBUG("Initializing Vulkan driver...");

  const auto& conf{Engine::Get().GetConfigurationManager()};

  vk::VulkanDriverDescr descr{};
  descr.name = conf.Get<std::string>("application_name");
  descr.width = conf.Get<u16>("rendering_window_width");
  descr.height = conf.Get<u16>("rendering_window_height");

  descr.is_specific_transfer_queue_requested =
      Engine::Get().GetConfigurationManager().Get<bool>(
          "rendering_vulkan_is_specific_transfer_queue_requested");

  descr.max_frames_in_flight = Engine::Get().GetConfigurationManager().Get<u8>(
      "rendering_vulkan_max_frames_in_flight");

  driver_ = std::make_unique<vk::VulkanDriver>(descr);
}

void RenderingManager::GenerateDirect3D12Driver() {
  COMET_LOG_RENDERING_DEBUG("Initializing Direct3D 12 driver...");

  throw std::runtime_error("DirectX 12 is unsupported at this time.");
}
}  // namespace rendering
}  // namespace comet
