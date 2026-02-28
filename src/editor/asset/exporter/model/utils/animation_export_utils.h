// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_MODEL_UTILS_ANIMATION_EXPORT_UTILS_H_
#define COMET_EDITOR_ASSET_EXPORTER_MODEL_UTILS_ANIMATION_EXPORT_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "assimp/anim.h"
#include "assimp/scene.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/animation/animation_common.h"
#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/map.h"
#include "comet/math/matrix.h"
#include "comet/resource/animation_resource.h"
#include "editor/asset/exporter/model/model_export.h"

namespace comet {
namespace editor {
namespace asset {
constexpr auto kDefaultAnimationFrameRate{30};
constexpr auto kDefaultTicksPerSecond{30.0f};

struct AnimationChannelDataNode {
  const aiNodeAnim* channel{nullptr};
  math::Mat4 baked_transform{1.0f};
  bool has_channel{false};
};

using AnimationChannelData =
    Map<geometry::SkeletonJointId, AnimationChannelDataNode>;

geometry::SkeletonJointId GenerateSkeletonJointId(const aiNode* node);
geometry::SkeletonJointId GenerateSkeletonJointId(const aiNodeAnim* node);
bool IsLoop(const schar* animation_name);
u32 ResolveAnimationKeyIndex(const aiVectorKey* vector_keys,
                             u32 vector_key_count, f64 tick_time);
u32 ResolveAnimationKeyIndex(const aiQuatKey* quat_keys, u32 quat_key_count,
                             f64 tick_time);
animation::JointPose ExtractPose(const aiNodeAnim* channel, f64 tick_time);
animation::JointPose ToJointPose(const math::Mat4& transform);
void PopulateMissingAnimationChannelDataNodes(
    const aiNode* node, AnimationChannelData& channel_data);
AnimationChannelData GenerateAnimationChannelData(
    ModelExport& model_export, const aiAnimation* raw_animation);
void PopulateSample(ModelExport& model_export,
                    const geometry::Skeleton& skeleton,
                    const AnimationChannelData& channel_data, u32 tick_delta,
                    animation::FrameIndex frame,
                    animation::CompressedAnimationSample& sample);
void PopulateAnimationClip(ModelExport& model_export,
                           const aiAnimation* raw_animation,
                           const geometry::Skeleton& skeleton,
                           resource::AnimationClipResource& clip_resource);
Array<resource::AnimationClipResource> LoadAnimationClips(
    ModelExport& model_export, const geometry::Skeleton& skeleton);
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_MODEL_UTILS_ANIMATION_EXPORT_UTILS_H_
