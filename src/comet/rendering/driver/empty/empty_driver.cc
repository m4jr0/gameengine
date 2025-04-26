// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "empty_driver.h"

#ifdef COMET_DEBUG

#include "comet/core/logger.h"
#include "comet/rendering/window/window.h"

namespace comet {
namespace rendering {
namespace empty {
EmptyDriver::EmptyDriver(const EmptyDriverDescr& descr) : Driver{descr} {
  WindowDescr window_descr{};
  window_descr.width = descr.window_width;
  window_descr.height = descr.window_height;
  SetName(window_descr, descr.app_name, descr.app_name_len);
  window_ = std::make_unique<EmptyGlfwWindow>(window_descr);
}

void EmptyDriver::Initialize() {
  Driver::Initialize();
  COMET_LOG_RENDERING_DEBUG("Initializing Empty driver.");
  window_->Initialize();
  COMET_ASSERT(window_->IsInitialized(), " GLFW window is not initialized!");
}

void EmptyDriver::Shutdown() {
  if (window_->IsInitialized()) {
    window_->Destroy();
  }

  Driver::Shutdown();
}

void EmptyDriver::Update(frame::FramePacket*) {}

DriverType EmptyDriver::GetType() const noexcept { return DriverType::Empty; }

void EmptyDriver::SetSize(WindowSize, WindowSize) {}

Window* EmptyDriver::GetWindow() { return window_.get(); }

u32 EmptyDriver::GetDrawCount() const { return 0; }
}  // namespace empty
}  // namespace rendering
}  // namespace comet

#endif  // COMET_DEBUG
