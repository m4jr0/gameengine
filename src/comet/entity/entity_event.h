// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_EVENT_H_
#define COMET_COMET_ENTITY_ENTITY_EVENT_H_

#include "comet/core/essentials.h"
#include "comet/core/type/string_id.h"
#include "comet/entity/entity_id.h"
#include "comet/event/event.h"

namespace comet {
namespace entity {
class ModelLoadedEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  ModelLoadedEvent(EntityId entity_id);
  ModelLoadedEvent(const ModelLoadedEvent&) = default;
  ModelLoadedEvent(ModelLoadedEvent&&) noexcept = default;
  ModelLoadedEvent& operator=(const ModelLoadedEvent&) = default;
  ModelLoadedEvent& operator=(ModelLoadedEvent&&) noexcept = default;
  virtual ~ModelLoadedEvent() = default;

  stringid::StringId GetType() const noexcept override;
  EntityId GetEntityId() const noexcept;

 private:
  EntityId entity_id_{kInvalidEntityId};
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_ENTITY_EVENT_H_
