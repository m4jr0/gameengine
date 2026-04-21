// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_MEMORY_LABEL_H_
#define COMET_COMET_CORE_MEMORY_MEMORY_LABEL_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace memory {
#ifdef COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
using GetCustomMemoryTagLabelFunc = const schar* (*)(MemoryTag);

namespace internal {
extern GetCustomMemoryTagLabelFunc get_custom_memory_tag_label_func;
}  // namespace internal

void AttachGetCustomMemoryTagLabelFunc(GetCustomMemoryTagLabelFunc func);
void DetachGetCustomMemoryTagLabelFunc();
#endif  // COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS

const schar* GetMemoryTagLabel(MemoryTag tag);
}  // namespace memory
}  // namespace comet

#ifdef COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
#define COMET_ATTACH_CUSTOM_MEMORY_LABEL_FUNC(func) \
  comet::memory::AttachGetCustomMemoryTagLabelFunc(func)
#define COMET_DETACH_CUSTOM_MEMORY_LABEL_FUNC() \
  comet::memory::DetachGetCustomMemoryTagLabelFunc()
#else
#define COMET_ATTACH_CUSTOM_MEMORY_LABEL_FUNC(func)
#define COMET_DETACH_CUSTOM_MEMORY_LABEL_FUNC()
#endif  // COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS

#endif  // COMET_COMET_CORE_MEMORY_MEMORY_LABEL_H_
