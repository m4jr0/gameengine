// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_DEBUGGING_UI_ENVIRONMENT_ENVIRONMENT_DEBUG_UI_H_
#define COMET_COMET_DEBUGGING_UI_ENVIRONMENT_ENVIRONMENT_DEBUG_UI_H_

#include "comet/core/essentials.h"
#include "comet/environment/environment_manager.h"

#ifdef COMET_HAS_DEBUG_UI

namespace comet {
namespace scene {
class EnvironmentManager;
}

namespace debug {
class EnvironmentDebugUi {
 public:
  EnvironmentDebugUi() = default;
  EnvironmentDebugUi(const EnvironmentDebugUi&) = delete;
  EnvironmentDebugUi(EnvironmentDebugUi&&) = delete;
  EnvironmentDebugUi& operator=(const EnvironmentDebugUi&) = delete;
  EnvironmentDebugUi& operator=(EnvironmentDebugUi&&) = delete;
  ~EnvironmentDebugUi() = default;

  void Draw(environment::EnvironmentManager& environment) const;
};
}  // namespace debug
}  // namespace comet

#endif  // COMET_HAS_DEBUG_UI

#endif  // COMET_COMET_DEBUGGING_UI_ENVIRONMENT_ENVIRONMENT_DEBUG_UI_H_