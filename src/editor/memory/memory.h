// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_MEMORY_MEMORY_H_
#define COMET_EDITOR_MEMORY_MEMORY_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace memory {
enum EditorMemoryTag : MemoryTag {
  kEditorMemoryTagAsset = kEngineMemoryTagUserBase + 1
};

#ifdef COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
const schar* GetEditorMemoryTagLabel(MemoryTag tag);
#endif  // COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
}  // namespace memory
}  // namespace comet

#endif  // COMET_EDITOR_MEMORY_MEMORY_H_
