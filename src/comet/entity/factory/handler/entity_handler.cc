// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "entity_handler.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace entity {
Handler::~Handler() {
  COMET_ASSERT(!is_initialized_, "entity::Handler::~Handler",
               "handler is still initialized");
}

void Handler::Initialize() {
  COMET_ASSERT(!is_initialized_, "entity::Handler::Initialize",
               "handler is already initialized");
  OnInitialize();
  is_initialized_ = true;
}

void Handler::Shutdown() {
  COMET_ASSERT(is_initialized_, "entity::Handler::Shutdown",
               "handler is not initialized");
  OnShutdown();
  is_initialized_ = false;
}

bool Handler::IsInitialized() const noexcept { return is_initialized_; }

void Handler::OnInitialize() {}

void Handler::OnShutdown() {}
}  // namespace entity
}  // namespace comet