#ifndef COMET_COMET_DEBUG_UI_ENVIRONMENT_ENVIRONMENT_DEBUG_UI_H_
#define COMET_COMET_DEBUG_UI_ENVIRONMENT_ENVIRONMENT_DEBUG_UI_H_

#include "comet/core/essentials.h"

#ifdef COMET_HAS_DEBUG_UI

namespace comet {
namespace scene {
class EnvironmentManager;
}

namespace debugui {
class EnvironmentDebugUi {
 public:
  EnvironmentDebugUi() = default;
  EnvironmentDebugUi(const EnvironmentDebugUi&) = delete;
  EnvironmentDebugUi(EnvironmentDebugUi&&) = delete;
  EnvironmentDebugUi& operator=(const EnvironmentDebugUi&) = delete;
  EnvironmentDebugUi& operator=(EnvironmentDebugUi&&) = delete;
  virtual ~EnvironmentDebugUi() = default;

  void Draw(scene::EnvironmentManager& environment) const;
};
}  // namespace debugui
}  // namespace comet

#endif  // COMET_HAS_DEBUG_UI

#endif  // COMET_COMET_DEBUG_UI_ENVIRONMENT_ENVIRONMENT_DEBUG_UI_H_