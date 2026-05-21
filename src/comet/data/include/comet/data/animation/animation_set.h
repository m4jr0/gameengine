// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_ANIMATION_ANIMATION_SET_H_
#define COMET_DATA_ANIMATION_ANIMATION_SET_H_

#include "comet/animation/animation_id.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/map.h"

namespace comet {
namespace animation {
// TODO(m4jr0): This should be generated from some resource at some point.
class AnimationSet {
 public:
  AnimationSet() = default;

  explicit AnimationSet(memory::Allocator* allocator, usize capacity = 0);

  void Reserve(usize capacity);

  void Set(const schar* local_name, AnimationClipId id);
  void Set(const schar* local_name, const schar* qualified_name);
  void Set(const schar* local_name, const wchar* qualified_name);

  AnimationClipId TryGet(const schar* local_name) const;
  AnimationClipId Get(const schar* local_name) const;

  bool IsEmpty() const noexcept;

 private:
  Map<resource::RawResourceId, AnimationClipId> animations_{};
};
}  // namespace animation
}  // namespace comet

#endif  // COMET_DATA_ANIMATION_ANIMATION_SET_H_