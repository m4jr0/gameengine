// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/animation/animation_id.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace animation {
AnimationClipId GenerateQualifiedAnimationClipId(CTStringView file_path,
                                                 const schar* animation_name) {
  COMET_ASSERT(animation_name != nullptr,
               "animation_id::GenerateQualifiedAnimationClipId",
               "animation name is null");
  COMET_ASSERT(!file_path.IsEmpty(),
               "animation_id::GenerateQualifiedAnimationClipId",
               "animation file path is empty");

  auto hash{BeginHash()};
  hash = ContinueHash(hash, file_path);
  hash = ContinueHash(hash, "|", 1);
  hash = ContinueHash(hash, animation_name);

  return AnimationClipId{EndHash(hash)};
}

AnimationClipId GenerateAnimationClipId(const schar* qualified_name) {
  COMET_ASSERT(qualified_name != nullptr,
               "animation_id::GenerateAnimationClipId",
               "animation qualified name is null");
  return AnimationClipId{COMET_STRING_ID(qualified_name)};
}

AnimationClipId GenerateAnimationClipId(const wchar* qualified_name) {
  COMET_ASSERT(qualified_name != nullptr,
               "animation_id::GenerateAnimationClipId",
               "animation qualified name is null");
  return AnimationClipId{COMET_STRING_ID(qualified_name)};
}

AnimationClipId GenerateAnimationClipId(CTStringView qualified_name) {
  COMET_ASSERT(!qualified_name.IsEmpty(),
               "animation_id::GenerateAnimationClipId",
               "animation qualified name is empty");
  return AnimationClipId{COMET_STRING_ID(qualified_name)};
}
}  // namespace animation
}  // namespace comet