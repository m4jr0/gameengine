// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/empty/empty_driver.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_DEBUG

#include "comet/core/logger/logging.h"
#include "comet/platform/window/window.h"

namespace comet {
namespace render {
namespace empty {
EmptyDriver::EmptyDriver(const EmptyDriverDescr& descr) : Driver{descr} {
  WindowDescr window_descr{};
  window_descr.width = descr.window_width;
  window_descr.height = descr.window_height;
  SetName(window_descr, descr.app_name, descr.app_name_len);
  window_ = std::make_unique<platform::EmptyGlfwWindow>(window_descr);
}

void EmptyDriver::Update(frame::FramePacket*) {}

DriverType EmptyDriver::GetType() const noexcept { return DriverType::Empty; }

void EmptyDriver::SetSize(WindowSize, WindowSize) {}

platform::Window* EmptyDriver::GetWindow() { return window_.get(); }

u32 EmptyDriver::GetDrawCount() const { return 0; }

void EmptyDriver::OnInitialize() {
  COMET_LOG_DEBUG(LoggerType::Rendering, "EmptyDriver::OnInitialize",
                  "initializing empty driver");
  COMET_ASSERT(window_ != nullptr, "EmptyDriver::OnInitialize",
               "window is null");
  window_->Initialize();
  COMET_ASSERT(window_->IsInitialized(), "EmptyDriver::OnInitialize",
               "window not initialized");
}

void EmptyDriver::OnShutdown() {
  if (window_->IsInitialized()) {
    window_->Destroy();
  }
}
}  // namespace empty
}  // namespace render
}  // namespace comet

#endif  // COMET_DEBUG
