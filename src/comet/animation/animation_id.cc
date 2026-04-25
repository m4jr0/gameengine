// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "animation_id.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_string.h"

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
  const auto animation_len{GetLength(animation_name)};

  const auto path_len{file_path.GetLength()};
  usize total_len{path_len + 1 + animation_len};

  constexpr auto kMaxStackBufferSize{512};
  schar* buffer{nullptr};
  schar stack_buffer[kMaxStackBufferSize];

  if (total_len + 1 <= kMaxStackBufferSize) {
    buffer = stack_buffer;
  } else {
    buffer = GenerateFrameString<schar>(total_len + 1);
  }

  Copy(buffer, file_path.GetCTStr(), path_len);
  buffer[path_len] = '|';
  Copy(buffer, animation_name, animation_len, path_len + 1);
  buffer[total_len] = '\0';

  return AnimationClipId{COMET_STRING_ID(buffer)};
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