// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/animation/animation_resource_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/container/array.h"
#include "comet/core/memory/memory_utils.h"

namespace comet {
namespace resource {
namespace internal {
void WriteCompressedJointPose(u8* buffer, usize& cursor,
                              const animation::CompressedJointPose& pose) {
  WriteValue(buffer, cursor, pose.rotation_x);
  WriteValue(buffer, cursor, pose.rotation_y);
  WriteValue(buffer, cursor, pose.rotation_z);
  WriteValue(buffer, cursor, pose.translation_x);
  WriteValue(buffer, cursor, pose.translation_y);
  WriteValue(buffer, cursor, pose.translation_z);
  WriteValue(buffer, cursor, pose.scale);
}


}  // namespace internal

AnimationClipResourceHandler::AnimationClipResourceHandler(
    const ResourceHandlerDescr& descr)
    : Base{descr} {}

void AnimationClipResourceHandler::OnInitialize() {
  anim_allocator_ = memory::FiberFreeListAllocator{
      kAnimAllocatorElementSize_, kDefaultAllocatorCapacity_,
      kEngineMemoryTagResourceAnimation};

  anim_allocator_.Initialize();
}

void AnimationClipResourceHandler::OnDestroy() { anim_allocator_.Destroy(); }

ResourceFile AnimationClipResourceHandler::Pack(const AnimationClipResource& resource,
    CompressionMode compression_mode) {
  return PackAnimationClipResource(resource, compression_mode, byte_allocator_);
}

void AnimationClipResourceHandler::Unpack(const ResourceFile& file,
    ResourceLifeSpan life_span, AnimationClipResource* resource) {
  UnpackAnimationClipResource(file, byte_allocator_, ResolveAllocator(&anim_allocator_, life_span), resource);
  resource->life_span = life_span;
}
}  // namespace resource
}  // namespace comet