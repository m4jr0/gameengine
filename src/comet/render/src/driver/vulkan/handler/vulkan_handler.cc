// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/handler/vulkan_handler.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
namespace vk {
Handler::Handler(const HandlerDescr& descr) : context_{descr.context} {
  COMET_ASSERT(context_ != nullptr, "Handler::Handler", "context is null");
}

Handler::~Handler() {
  COMET_ASSERT(!is_initialized_, "Handler::~Handler",
               "handler is still initialized");
}

void Handler::Initialize() {
  COMET_ASSERT(!is_initialized_, "Handler::Initialize",
               "handler is already initialized");
  OnInitialize();
  is_initialized_ = true;
}

void Handler::Shutdown() {
  COMET_ASSERT(is_initialized_, "Handler::Shutdown",
               "handler is not initialized");
  OnShutdown();
  is_initialized_ = false;
}

bool Handler::IsInitialized() const noexcept { return is_initialized_; }

void Handler::OnInitialize() {}

void Handler::OnShutdown() {}
}  // namespace vk
}  // namespace rendering
}  // namespace comet