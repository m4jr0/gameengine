#ifndef COMET_COMET_DEBUG_UI_DEBUG_UI_MANAGER_H_
#define COMET_COMET_DEBUG_UI_DEBUG_UI_MANAGER_H_

#include "comet/core/essentials.h"

#ifdef COMET_HAS_DEBUG_UI

#include "comet/core/manager.h"
#include "comet/debug_ui/environment/environment_debug_ui.h"
#include "comet/rendering/debug_ui_registry.h"

#ifdef COMET_HAS_PROFILER_DEBUG_UI
#include "comet/debug_ui/debugger/debugger_debug_ui.h"
#endif  // COMET_HAS_PROFILER_DEBUG_UI

namespace comet {
namespace debugui {
class DebugUiManager : public Manager {
 public:
  static DebugUiManager& Get();

  DebugUiManager() = default;
  DebugUiManager(const DebugUiManager&) = delete;
  DebugUiManager(DebugUiManager&&) = delete;
  DebugUiManager& operator=(const DebugUiManager&) = delete;
  DebugUiManager& operator=(DebugUiManager&&) = delete;
  virtual ~DebugUiManager() = default;

  void Initialize() override;
  void Shutdown() override;

 private:
  rendering::DebugUiRegistry::CallbackId environment_callback_id_{
      rendering::DebugUiRegistry::kInvalidCallbackId};

#ifdef COMET_HAS_PROFILER_DEBUG_UI
  rendering::DebugUiRegistry::CallbackId debugger_callback_id_{
      rendering::DebugUiRegistry::kInvalidCallbackId};
  DebuggerDebugUi debugger_debug_ui_{};
#endif  // COMET_HAS_PROFILER_DEBUG_UI

  EnvironmentDebugUi environment_debug_ui_{};
};
}  // namespace debugui
}  // namespace comet

#endif  // COMET_HAS_DEBUG_UI

#endif  // COMET_COMET_DEBUG_UI_DEBUG_UI_MANAGER_H_