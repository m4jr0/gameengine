// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_SCENE_SCENE_EVENT_H_
#define COMET_COMET_SCENE_SCENE_EVENT_H_

#include "comet/core/essentials.h"
#include "comet/core/type/string_id.h"
#include "comet/event/event.h"

namespace comet {
namespace scene {
class SceneLoadRequestEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  explicit SceneLoadRequestEvent() = default;
  SceneLoadRequestEvent(const SceneLoadRequestEvent&) = default;
  SceneLoadRequestEvent(SceneLoadRequestEvent&&) noexcept = default;
  SceneLoadRequestEvent& operator=(const SceneLoadRequestEvent&) = default;
  SceneLoadRequestEvent& operator=(SceneLoadRequestEvent&&) noexcept = default;
  virtual ~SceneLoadRequestEvent() = default;

  stringid::StringId GetType() const noexcept override;
};

class SceneLoadedEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  SceneLoadedEvent() = default;
  SceneLoadedEvent(const SceneLoadedEvent&) = default;
  SceneLoadedEvent(SceneLoadedEvent&&) noexcept = default;
  SceneLoadedEvent& operator=(const SceneLoadedEvent&) = default;
  SceneLoadedEvent& operator=(SceneLoadedEvent&&) noexcept = default;
  virtual ~SceneLoadedEvent() = default;

  stringid::StringId GetType() const noexcept override;
};

class SceneUnloadedEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  SceneUnloadedEvent() = default;
  SceneUnloadedEvent(const SceneUnloadedEvent&) = default;
  SceneUnloadedEvent(SceneUnloadedEvent&&) noexcept = default;
  SceneUnloadedEvent& operator=(const SceneUnloadedEvent&) = default;
  SceneUnloadedEvent& operator=(SceneUnloadedEvent&&) noexcept = default;
  virtual ~SceneUnloadedEvent() = default;

  stringid::StringId GetType() const noexcept override;
};
}  // namespace scene
}  // namespace comet

#endif  // COMET_COMET_SCENE_SCENE_EVENT_H_
