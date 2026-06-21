// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_MEMORY_MEMORY_TAG_H_
#define COMET_RUNTIME_MEMORY_MEMORY_TAG_H_

// External. ///////////////////////////////////////////////////////////////////
#include <cstddef>
#include <functional>
#include <memory>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"

namespace comet {
namespace memory {
using MemoryTag = u64;

enum EngineMemoryTag : MemoryTag {
  kEngineMemoryTagUntagged = 0,
  kEngineMemoryTagCore,
  kEngineMemoryTagConfig,
  kEngineMemoryTagTaggedHeap,
  kEngineMemoryTagGid,
  kEngineMemoryTagFrame,
  kEngineMemoryTagDoubleFrame,
  kEngineMemoryTagFrameExtended,
  kEngineMemoryTagDoubleFrameExtended1,
  kEngineMemoryTagDoubleFrameExtended2,
  kEngineMemoryTagContinuation,
  kEngineMemoryTagGeometry,
  kEngineMemoryTagRender,
  kEngineMemoryTagRenderInternal,
  kEngineMemoryTagRenderDevice,
  kEngineMemoryTagResource,
  kEngineMemoryTagResourceGlobal,
  kEngineMemoryTagResourceGlobalExtended,
  kEngineMemoryTagResourceScene,
  kEngineMemoryTagResourceSceneExtended,
  kEngineMemoryTagResourceAnimationHandler,
  kEngineMemoryTagResourceMaterialHandler,
  kEngineMemoryTagResourceStaticModelHandler,
  kEngineMemoryTagResourceSkeletalModelHandler,
  kEngineMemoryTagResourceSkeletonHandler,
  kEngineMemoryTagResourceAnimationClipHandler,
  kEngineMemoryTagResourceShaderModuleHandler,
  kEngineMemoryTagResourceShaderHandler,
  kEngineMemoryTagResourceTextureHandler,
  kEngineMemoryTagResourceAnimation,
  kEngineMemoryTagResourceTexture,
  kEngineMemoryTagEntity,
  kEngineMemoryTagPendingEntity1,
  kEngineMemoryTagPendingEntity1Extended,
  kEngineMemoryTagPendingEntity2,
  kEngineMemoryTagPendingEntity2Extended,
  kEngineMemoryTagFiber,
  kEngineMemoryTagEvent,
  kEngineMemoryTagDebug,
  kEngineMemoryTagMainThread,
  kEngineMemoryTagUserBase = kU32Max,
  kEngineMemoryTagInvalid = kU64Max
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_RUNTIME_MEMORY_MEMORY_TAG_H_
