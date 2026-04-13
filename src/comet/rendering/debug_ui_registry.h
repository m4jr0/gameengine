#ifndef COMET_COMET_RENDERING_DEBUG_UI_REGISTRY_H_
#define COMET_COMET_RENDERING_DEBUG_UI_REGISTRY_H_

#include "comet/core/essentials.h"

#ifdef COMET_IMGUI

#include <functional>

#include "comet/core/manager.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"

namespace comet {
namespace rendering {
class DebugUiRegistry : public Manager {
 public:
  using DrawCallback = std::function<void()>;
  using CallbackId = u64;

  static inline constexpr CallbackId kInvalidCallbackId{0};

  static DebugUiRegistry& Get();

  void Initialize();
  void Destroy();

  CallbackId Register(DrawCallback callback);
  bool Unregister(CallbackId id);
  void Clear();
  void Draw() const;

 private:
  struct Entry {
    CallbackId id{kInvalidCallbackId};
    DrawCallback callback{};
  };

  CallbackId next_callback_id_{1};
  memory::PlatformAllocator allocator_{memory::kEngineMemoryTagDebug};
  Array<Entry> entries_{};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_IMGUI

#endif  // COMET_COMET_RENDERING_DEBUG_UI_REGISTRY_H_