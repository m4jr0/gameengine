// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "memory.h"

namespace comet {
namespace memory {
#ifdef COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
const schar* GetEditorMemoryTagLabel(comet::memory::MemoryTag tag) {
  switch (tag) {
    case kEditorMemoryTagAsset:
      return "asset";
    default:
      return nullptr;
  }
}
#endif  // COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
}  // namespace memory
}  // namespace comet