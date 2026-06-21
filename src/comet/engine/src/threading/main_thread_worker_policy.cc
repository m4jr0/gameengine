// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_engine_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/engine/threading/main_thread_worker_policy.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/render/utils/driver_utils.h"
#include "comet/runtime/conf/conf_manager.h"
#include "comet/runtime/conf/config_keys.h"

namespace comet {
namespace engine {
bool IsMainThreadWorkerDisabled() {
  const bool is_disabled{
      COMET_CONF_BOOL(conf::kCoreIsMainThreadWorkerDisabled)};

  const auto* driver_label{COMET_CONF_STR(conf::kRenderingDriver)};
  const auto driver_type{render::GetDriverTypeFromStr(driver_label)};
  const bool is_rendering_multithreaded{render::IsMultithreading(driver_type)};

  return !is_rendering_multithreaded || is_disabled;
}
}  // namespace engine
}  // namespace comet