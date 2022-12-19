// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_handler.h"

namespace comet {
namespace rendering {
namespace vk {
Handler::Handler(const HandlerDescr& descr) : context_{descr.context} {
  COMET_ASSERT(context_ != nullptr, "Context cannot be null for handler!");
}

Handler ::~Handler() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for handler, but it is still initialized!");
}

void Handler ::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize handler, but it is already done!");
  is_initialized_ = true;
}

void Handler ::Shutdown() {
  COMET_ASSERT(is_initialized_,
               "Tried to shutdown handler, but it is not initialized!");
  is_initialized_ = false;
}

bool Handler::IsInitialized() const noexcept { return is_initialized_; }
}  // namespace vk
}  // namespace rendering
}  // namespace comet