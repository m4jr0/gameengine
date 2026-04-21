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
}  // namespace memory
}  // namespace comet

#endif  // COMET_EDITOR_MEMORY_MEMORY_H_
