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
Handler::Handler(const HandlerDescr&) {}

Handler::~Handler() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for handler, but it is still initialized!");
}

void Handler::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize handler, but it is already done!");
  is_initialized_ = true;
}

void Handler::Shutdown() {
  COMET_ASSERT(is_initialized_,
               "Tried to shutdown handler, but it is not initialized!");
  is_initialized_ = false;
}

bool Handler::IsInitialized() const noexcept { return is_initialized_; }
}  // namespace gl
}  // namespace rendering
}  // namespace comet