// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_RESOURCE_ANIMATION_ANIMATION_RESOURCE_HANDLE_H_
#define COMET_RUNTIME_RESOURCE_ANIMATION_ANIMATION_RESOURCE_HANDLE_H_

#include "comet/data/resource/animation/animation_resource.h"
#include "comet/runtime/resource/loaded_resource_handle.h"

namespace comet {
namespace resource {
using AnimationClipResourceHandle =
    LoadedResourceHandle<AnimationClipResourceTag>;
}  // namespace resource
}  // namespace comet

#endif  // COMET_RUNTIME_RESOURCE_ANIMATION_ANIMATION_RESOURCE_HANDLE_H_