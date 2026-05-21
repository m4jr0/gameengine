// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/engine/threading/main_thread_worker_policy.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"
#include "comet/rendering/utils/driver_utils.h"

namespace comet { // >:3 Check moved function here.
#ifdef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
static const bool result = [] {
  const bool is_disabled{
      COMET_CONF_BOOL(conf::kCoreIsMainThreadWorkerDisabled)};
  const auto* driver_label{COMET_CONF_STR(conf::kRenderingDriver)};
  const auto driver_type{rendering::GetDriverTypeFromStr(driver_label)};
  const bool is_rendering_multithreaded{
      rendering::IsMultithreading(driver_type)};

  return !is_rendering_multithreaded || is_disabled;
}();
return result;
#else
return false;
#endif  // COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
}  // namespace comet
