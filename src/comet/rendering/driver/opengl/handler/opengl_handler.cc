// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_handler.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
namespace gl {
Handler::Handler(const HandlerDescr& descr) : frame_state_{descr.frame_state} {
  COMET_ASSERT(frame_state_ != nullptr,
               "Frame state cannot be null for handler!");
}

Handler::~Handler() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for handler, but it is still initialized!");
}

void Handler::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize handler, but it is already done!");
  OnInitialize();
  is_initialized_ = true;
}

void Handler::Shutdown() {
  COMET_ASSERT(is_initialized_,
               "Tried to shutdown handler, but it is not initialized!");
  OnShutdown();
  is_initialized_ = false;
}

bool Handler::IsInitialized() const noexcept { return is_initialized_; }

void Handler::OnInitialize() {}

void Handler::OnShutdown() {}
}  // namespace gl
}  // namespace rendering
}  // namespace comet