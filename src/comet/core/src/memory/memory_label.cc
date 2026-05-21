// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/memory/memory_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace memory {
#ifdef COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
namespace internal {
GetCustomMemoryTagLabelFunc get_custom_memory_tag_label_func{nullptr};
}  // namespace internal

void AttachGetCustomMemoryTagLabelFunc(GetCustomMemoryTagLabelFunc func) {
  COMET_ASSERT(func != nullptr,
               "memory_label::AttachGetCustomMemoryTagLabelFunc",
               "custom memory tag label function is null");
  internal::get_custom_memory_tag_label_func = func;
}

void DetachGetCustomMemoryTagLabelFunc() {
  internal::get_custom_memory_tag_label_func = nullptr;
}
#endif  // COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS

const schar* GetMemoryTagLabel(MemoryTag tag) {
  switch (tag) {
    case kEngineMemoryTagUntagged:
      return "untagged";
    case kEngineMemoryTagConfig:
      return "config";
    case kEngineMemoryTagCore:
      return "core";
    case kEngineMemoryTagTaggedHeap:
      return "tagged_heap";
    case kEngineMemoryTagGid:
      return "gid";
    case kEngineMemoryTagStringId:
      return "string_id";
    case kEngineMemoryTagFrame:
      return "frame";
    case kEngineMemoryTagInput:
      return "input";
    case kEngineMemoryTagFrameExtended:
      return "frame_extended";
    case kEngineMemoryTagDoubleFrame:
      return "double_frame";
    case kEngineMemoryTagDoubleFrameExtended1:
      return "double_frame_extended_1";
    case kEngineMemoryTagDoubleFrameExtended2:
      return "double_frame_extended_2";
    case kEngineMemoryTagContinuation:
      return "continuation";
    case kEngineMemoryTagGeometry:
      return "geometry";
    case kEngineMemoryTagRendering:
      return "rendering";
    case kEngineMemoryTagRenderingInternal:
      return "rendering_internal";
    case kEngineMemoryTagRenderingDevice:
      return "rendering_device (VRAM)";
    case kEngineMemoryTagResource:
      return "resource";
    case kEngineMemoryTagResourceGlobal:
      return "resource_global";
    case kEngineMemoryTagResourceGlobalExtended:
      return "resource_global_extended";
    case kEngineMemoryTagResourceScene:
      return "resource_scene";
    case kEngineMemoryTagResourceSceneExtended:
      return "resource_scene_extended";
    case kEngineMemoryTagResourceAnimationHandler:
      return "resource_animation_handler";
    case kEngineMemoryTagResourceMaterialHandler:
      return "resource_material_handler";
    case kEngineMemoryTagResourceStaticModelHandler:
      return "resource_static_model_handler";
    case kEngineMemoryTagResourceSkeletalModelHandler:
      return "resource_skeletal_model_handler";
    case kEngineMemoryTagResourceSkeletonHandler:
      return "resource_skeleton_handler";
    case kEngineMemoryTagResourceAnimationClipHandler:
      return "resource_animation_clip_handler";
    case kEngineMemoryTagResourceShaderModuleHandler:
      return "resource_shader_module_handler";
    case kEngineMemoryTagResourceShaderHandler:
      return "resource_shader_handler";
    case kEngineMemoryTagResourceTextureHandler:
      return "resource_texture_handler";
    case kEngineMemoryTagResourceAnimation:
      return "resource_animation";
    case kEngineMemoryTagResourceTexture:
      return "resource_texture";
    case kEngineMemoryTagTString:
      return "tstring";
    case kEngineMemoryTagEntity:
      return "entity";
    case kEngineMemoryTagPendingEntity1:
      return "pending_entity_1";
    case kEngineMemoryTagPendingEntity1Extended:
      return "pending_entity_1_extended";
    case kEngineMemoryTagPendingEntity2:
      return "pending_entity_2";
    case kEngineMemoryTagPendingEntity2Extended:
      return "pending_entity_2_extended";
    case kEngineMemoryTagFiber:
      return "fiber";
    case kEngineMemoryTagThreadProvider:
      return "thread_provider";
    case kEngineMemoryTagEvent:
      return "event";
    case kEngineMemoryTagDebug:
      return "debug";
    case kEngineMemoryTagMainThread:
      return "main_thread";
    case kEngineMemoryTagUserBase:
      return "user_base (should not be used)";
    case kEngineMemoryTagInvalid:
      return "invalid (please investigate)";
#ifdef COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
    default:
      if (internal::get_custom_memory_tag_label_func != nullptr) {
        const auto* result{internal::get_custom_memory_tag_label_func(tag)};

        if (result != nullptr) {
          return result;
        }
      }
#endif  // COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
  }

  return kUnknownLabel;
}
}  // namespace memory
}  // namespace comet
