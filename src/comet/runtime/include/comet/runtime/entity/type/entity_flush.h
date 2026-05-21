// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ENTITY_TYPE_ENTITY_FLUSH_H_
#define COMET_RUNTIME_ENTITY_TYPE_ENTITY_FLUSH_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_container.h"
#include "comet/entity/type/archetype.h"
#include "comet/entity/type/entity_id.h"
#include "comet/entity/type/pending_entity.h"

namespace comet {
namespace entity {
namespace internal {
struct EntityMovePlan {
  EntityId entity_id{kInvalidEntityId};

  Archetype* old_archetype{nullptr};
  usize old_row{kInvalidIndex};

  Archetype* new_archetype{nullptr};
  usize new_row{kInvalidIndex};

  const PendingEntity* pending{nullptr};
};

struct EntityInPlaceUpdatePlan {
  EntityId entity_id;
  Archetype* archetype;
  usize row;
  const PendingEntity* pending;
};

struct RowMove {
  usize dst_row{kInvalidIndex};
  usize src_row{kInvalidIndex};
  EntityId moved_entity_id{kInvalidEntityId};
};

struct ArchetypePlan {
  Archetype* archetype;

  usize old_size{0};
  usize add_count{0};
  usize remove_count{0};
  usize final_size{0};

  frame::FrameArray<usize> incoming{};
  frame::FrameArray<usize> outgoing{};
  frame::FrameArray<RowMove> compaction_moves{};
  frame::FrameArray<usize> destroyed_rows{};
};

struct FlushPlan {
  usize max_staging_archetype_size{0};

  frame::FrameArray<EntityMovePlan> entity_moves{};
  frame::FrameArray<EntityInPlaceUpdatePlan> entity_in_place_updates{};
  frame::FrameArray<EntityId> destroyed_ids{};
  frame::FrameArray<ArchetypePlan> archetype_plans{};

  bool IsEmpty() const;
};
}  // namespace internal
}  // namespace entity
}  // namespace comet

#endif  // COMET_RUNTIME_ENTITY_TYPE_ENTITY_FLUSH_H_
