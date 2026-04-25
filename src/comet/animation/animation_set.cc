// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "animation_set.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace animation {
AnimationSet::AnimationSet(memory::Allocator* allocator, usize capacity) {
  animations_ = Map<resource::RawResourceId, AnimationClipId>::WithCapacity(
      allocator, capacity);
}

void AnimationSet::Reserve(usize capacity) { animations_.Reserve(capacity); }

void AnimationSet::Set(const schar* local_name, AnimationClipId id) {
  COMET_ASSERT(local_name != nullptr, "AnimationSet::Set",
               "animation local name is null");
  COMET_ASSERT(local_name[0] != '\0', "AnimationSet::Set",
               "animation local name is empty");
  COMET_ASSERT(id, "AnimationSet::Set", "animation clip id is invalid");

  animations_.Set(COMET_STRING_ID(local_name), id);
}

void AnimationSet::Set(const schar* local_name, const schar* qualified_name) {
  COMET_ASSERT(qualified_name != nullptr, "AnimationSet::Set",
               "animation qualified name is null");
  Set(local_name, GenerateAnimationClipId(qualified_name));
}

void AnimationSet::Set(const schar* local_name, const wchar* qualified_name) {
  COMET_ASSERT(qualified_name != nullptr, "AnimationSet::Set",
               "animation qualified name is null");
  Set(local_name, GenerateAnimationClipId(qualified_name));
}

AnimationClipId AnimationSet::TryGet(const schar* local_name) const {
  COMET_ASSERT(local_name != nullptr, "AnimationSet::TryGet",
               "animation local name is null");

  COMET_ASSERT(local_name[0] != '\0', "AnimationSet::TryGet",
               "animation local name is empty");

  const auto* id_ptr{animations_.TryGet(COMET_STRING_ID(local_name))};
  return id_ptr == nullptr ? AnimationClipId::Invalid() : *id_ptr;
}

AnimationClipId AnimationSet::Get(const schar* local_name) const {
  const auto id{TryGet(local_name)};
  COMET_ASSERT(id, "AnimationSet::Get", "animation local name is unknown",
               "local_name", local_name);
  return id;
}

bool AnimationSet::IsEmpty() const noexcept { return animations_.IsEmpty(); }
}  // namespace animation
}  // namespace comet