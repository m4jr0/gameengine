// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_EVENT_RUNTIME_EVENT_H_
#define COMET_COMET_EVENT_RUNTIME_EVENT_H_

#include "comet_precompile.h"

#include "comet/event/event.h"

namespace comet {
namespace event {
class UnrecoverableErrorEvent : public Event {
 public:
  const static stringid::StringId kStaticType_;

  UnrecoverableErrorEvent() = default;
  UnrecoverableErrorEvent(const UnrecoverableErrorEvent&) = default;
  UnrecoverableErrorEvent(UnrecoverableErrorEvent&&) noexcept = default;
  UnrecoverableErrorEvent& operator=(const UnrecoverableErrorEvent&) = default;
  UnrecoverableErrorEvent& operator=(UnrecoverableErrorEvent&&) noexcept =
      default;
  ~UnrecoverableErrorEvent() = default;

  stringid::StringId GetType() const noexcept override;
};
}  // namespace event
}  // namespace comet

#endif  // COMET_COMET_EVENT_RUNTIME_EVENT_H_
