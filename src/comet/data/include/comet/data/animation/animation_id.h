// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_ANIMATION_ANIMATION_ID_H_
#define COMET_DATA_ANIMATION_ANIMATION_ID_H_

#include "comet/core/essentials.h"
#include "comet/core/string/tstring.h"
#include "comet/data/resource/resource_id.h"
#include "comet/data/resource/runtime/loaded_resource_handle.h"

namespace comet {
namespace animation {
struct AnimationClipTag {};

using AnimationClipId = resource::ResourceIdT<AnimationClipTag>;

AnimationClipId GenerateQualifiedAnimationClipId(CTStringView file_path,
                                                 const schar* animation_name);

// qualified_name format: "<resource_path>|<animation_name>".
AnimationClipId GenerateAnimationClipId(const schar* qualified_name);
AnimationClipId GenerateAnimationClipId(const wchar* qualified_name);
AnimationClipId GenerateAnimationClipId(CTStringView qualified_name);
}  // namespace animation
}  // namespace comet

#endif  // COMET_DATA_ANIMATION_ANIMATION_ID_H_
