// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENGINE_ENGINE_EVENT_H_
#define COMET_COMET_ENGINE_ENGINE_EVENT_H_

#include "comet/core/essentials.h"
#include "comet/core/type/string_id.h"
#include "comet/event/event.h"

namespace comet {
class ApplicationQuitEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  ApplicationQuitEvent() = default;
  ApplicationQuitEvent(const ApplicationQuitEvent&) = default;
  ApplicationQuitEvent(ApplicationQuitEvent&&) noexcept = default;
  ApplicationQuitEvent& operator=(const ApplicationQuitEvent&) = default;
  ApplicationQuitEvent& operator=(ApplicationQuitEvent&&) noexcept = default;
  virtual ~ApplicationQuitEvent() = default;

  stringid::StringId GetType() const noexcept override;
};

class UnrecoverableErrorEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  UnrecoverableErrorEvent() = default;
  UnrecoverableErrorEvent(const UnrecoverableErrorEvent&) = default;
  UnrecoverableErrorEvent(UnrecoverableErrorEvent&&) noexcept = default;
  UnrecoverableErrorEvent& operator=(const UnrecoverableErrorEvent&) = default;
  UnrecoverableErrorEvent& operator=(UnrecoverableErrorEvent&&) noexcept =
      default;
  virtual ~UnrecoverableErrorEvent() = default;

  stringid::StringId GetType() const noexcept override;
};
}  // namespace comet

#endif  // COMET_COMET_ENGINE_ENGINE_EVENT_H_
