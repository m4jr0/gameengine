// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "debug_ui_registry.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_IMGUI

namespace comet {
namespace rendering {
DebugUiRegistry& DebugUiRegistry::Get() {
  static DebugUiRegistry singleton{};
  return singleton;
}

void DebugUiRegistry::Initialize() {
  entries_ = Array<Entry>{&allocator_};
  entries_.Reserve(5);
}

void DebugUiRegistry::Destroy() { entries_.Destroy(); }

DebugUiRegistry::CallbackId DebugUiRegistry::Register(DrawCallback callback) {
  if (!callback) {
    return kInvalidCallbackId;
  }

  auto id{next_callback_id_++};

  auto& entry{entries_.EmplaceBack()};
  entry.id = id;
  entry.callback = std::move(callback);

  return id;
}

bool DebugUiRegistry::Unregister(CallbackId id) {
  if (id == kInvalidCallbackId) {
    return false;
  }

  for (usize i{0}; i < entries_.GetSize(); ++i) {
    if (entries_[i].id != id) {
      continue;
    }

    entries_.RemoveFromIndex(i);
    return true;
  }

  return false;
}

void DebugUiRegistry::Clear() { entries_.Clear(); }

void DebugUiRegistry::Draw() const {
  for (const auto& entry : entries_) {
    if (entry.callback) {
      entry.callback();
    }
  }
}
}  // namespace rendering
}  // namespace comet

#endif  // COMET_IMGUI