// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DEBUG_UI_REGISTRY_H_
#define COMET_RENDER_DEBUG_UI_REGISTRY_H_

#include "comet/core/essentials.h"

#ifdef COMET_IMGUI

// External. ///////////////////////////////////////////////////////////////////
#include <functional>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"

namespace comet {
namespace rendering {
class DebugUiRegistry {
 public:
  using DrawCallback = std::function<void()>;
  using CallbackId = u64;

  static inline constexpr CallbackId kInvalidCallbackId{0};

  static DebugUiRegistry& Get();

  ~DebugUiRegistry();

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

  bool is_initialized_{false};
  CallbackId next_callback_id_{1};
  memory::PlatformAllocator allocator_{memory::kEngineMemoryTagDebug};
  Array<Entry> entries_{};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_IMGUI

#endif  // COMET_RENDER_DEBUG_UI_REGISTRY_H_