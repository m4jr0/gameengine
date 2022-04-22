// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "rendering_manager.h"

#include "boost/format.hpp"

#include "comet/core/engine.h"
#include "comet/rendering/driver/opengl/opengl_driver.h"
#include "comet/rendering/driver/vulkan/vulkan_driver.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace rendering {
void RenderingManager::Initialize() {
  const auto rendering_driver =
      core::Engine::GetEngine().GetConfigurationManager().Get<std::string>(
          "rendering_driver");

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

void RenderingManager::Update(
    time::Interpolation interpolation,
    game_object::GameObjectManager& game_object_manager) {
  current_time_ += core::Engine::GetEngine().GetTimeManager().GetTimeDelta();

  if (current_time_ > 1000) {
    current_time_ = 0;
    counter_ = 0;
  }

  driver_->Update(interpolation, game_object_manager);
  core::Engine::GetEngine().GetInputManager().Update();
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

  const auto& conf = core::Engine::GetEngine().GetConfigurationManager();

  gl::OpenGlDriverDescr descr{};
  descr.name = conf.Get<std::string>("application_name");
  descr.width = conf.Get<unsigned int>("rendering_window_width");
  descr.height = conf.Get<unsigned int>("rendering_window_height");
  descr.major_version =
      conf.Get<unsigned int>("rendering_opengl_major_version");
  descr.minor_version =
      conf.Get<unsigned int>("rendering_opengl_minor_version");

  driver_ = std::make_unique<gl::OpenGlDriver>(descr);
}

void RenderingManager::GenerateVulkanDriver() {
  COMET_LOG_RENDERING_DEBUG("Initializing Vulkan driver...");

  const auto& conf = core::Engine::GetEngine().GetConfigurationManager();

  vk::VulkanDriverDescr descr{};
  descr.name = conf.Get<std::string>("application_name");
  descr.width = conf.Get<unsigned int>("rendering_window_width");
  descr.height = conf.Get<unsigned int>("rendering_window_height");

  driver_ = std::make_unique<vk::VulkanDriver>(descr);
}

void RenderingManager::GenerateDirect3D12Driver() {
  COMET_LOG_RENDERING_DEBUG("Initializing Direct3D 12 driver...");

  throw std::runtime_error("DirectX 12 is unsupported at this time.");
}
}  // namespace rendering
}  // namespace comet
