// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "rendering_manager.h"

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/engine.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/driver/opengl/opengl_driver.h"
#include "comet/rendering/driver/vulkan/vulkan_driver.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
void RenderingManager::Initialize() {
  const auto driver_type{
      GetDriverTypeFromTypeName(COMET_CONF_STR(conf::kRenderingDriver))};

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
}

void RenderingManager::Destroy() { driver_->Destroy(); }

void RenderingManager::Update(time::Interpolation interpolation,
                              entity::EntityManager& entity_manager) {
  current_time_ += Engine::Get().GetTimeManager().GetDeltaTime();

  if (current_time_ > 1000) {
    COMET_LOG_RENDERING_DEBUG(counter_, " FPS.");
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
  driver_ = std::make_unique<gl::OpenGlDriver>();
}

void RenderingManager::GenerateVulkanDriver() {
  COMET_LOG_RENDERING_DEBUG("Initializing Vulkan driver...");
  driver_ = std::make_unique<vk::VulkanDriver>();
}

void RenderingManager::GenerateDirect3D12Driver() {
  COMET_LOG_RENDERING_DEBUG("Initializing Direct3D 12 driver...");
  COMET_ASSERT(false, "DirectX 12 is unsupported at this time.");
}
}  // namespace rendering
}  // namespace comet
