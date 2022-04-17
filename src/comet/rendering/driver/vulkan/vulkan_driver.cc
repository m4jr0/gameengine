// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_driver.h"

#include "boost/format.hpp"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

#include "comet/core/engine.h"
#include "comet/event/event_manager.h"
#include "comet/event/input_event.h"
#include "comet/event/runtime_event.h"
#include "comet/event/window_event.h"

namespace comet {
namespace rendering {
namespace vk {
VulkanDriver::VulkanDriver(const VulkanDriverDescr& descr) {
  WindowDescr window_descr{};
  window_descr.width = descr.width;
  window_descr.height = descr.height;
  window_descr.name = descr.name;
  window_ = GlfwWindow(window_descr);
}

void VulkanDriver::Initialize() {
  window_.Initialize();

  if (!window_.IsInitialized()) {
    return;
  }

  event::EventManager& event_manager =
      core::Engine::GetEngine().GetEventManager();

  event_manager.Register(COMET_EVENT_BIND_FUNCTION(VulkanDriver::OnEvent),
                         event::WindowResizeEvent::kStaticType_);

  // Code.

  is_initialized_ = true;
}

void VulkanDriver::Destroy() {
  // Code.
  window_.Destroy();
}

void VulkanDriver::Start() {
  // Code.
}

void VulkanDriver::Update(time::Interpolation interpolation,
                          game_object::GameObjectManager& game_object_manager) {
  // Code.
}

void VulkanDriver::SetSize(unsigned int width, unsigned int height) {
  window_.SetSize(width, height);

  // Code.
}

void VulkanDriver::OnEvent(const event::Event& event) {
  const auto& event_type = event.GetType();

  if (event_type == event::WindowResizeEvent::kStaticType_) {
    const auto& window_event =
        static_cast<const event::WindowResizeEvent&>(event);
    SetSize(window_event.GetWidth(), window_event.GetHeight());
  }
}

bool VulkanDriver::IsInitialized() const { return is_initialized_; }

Window& VulkanDriver::GetWindow() { return window_; }
}  // namespace vk
}  // namespace rendering
}  // namespace comet
