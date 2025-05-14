// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_ANIMATION_ANIMATION_EXPORTER_UTILS_H_
#define COMET_EDITOR_ASSET_EXPORTER_ANIMATION_ANIMATION_EXPORTER_UTILS_H_

#include "assimp/scene.h"
#include "comet/animation/animation_common.h"
#include "comet/resource/animation_resource.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/resource/resource.h"

namespace comet {
namespace editor {
namespace asset {
bool IsLoop(const schar* animation_name);
u32 ResolveKeyIndex(const aiVectorKey* vector_keys, u32 vector_key_count,
                    f64 time);
u32 ResolveKeyIndex(const aiQuatKey* quat_keys, u32 quat_key_count, f64 time);
animation::JointPose ExtractPose(const aiNodeAnim* channel, f64 time);
Array<resource::AnimationClipResource> LoadAnimationClips(
    memory::Allocator* allocator, const aiScene* scene);
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_ANIMATION_ANIMATION_EXPORTER_UTILS_H_
