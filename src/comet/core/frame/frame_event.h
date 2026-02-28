// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_FRAME_EVENT_H_
#define COMET_COMET_CORE_FRAME_EVENT_H_

#include "comet/core/essentials.h"
#include "comet/core/type/string_id.h"
#include "comet/event/event.h"

namespace comet {
namespace frame {
class NewFrameEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  NewFrameEvent() = default;
  NewFrameEvent(const NewFrameEvent&) = default;
  NewFrameEvent(NewFrameEvent&&) noexcept = default;
  NewFrameEvent& operator=(const NewFrameEvent&) = default;
  NewFrameEvent& operator=(NewFrameEvent&&) noexcept = default;
  virtual ~NewFrameEvent() = default;

  stringid::StringId GetType() const noexcept override;
};

class EndFrameEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  EndFrameEvent() = default;
  EndFrameEvent(const EndFrameEvent&) = default;
  EndFrameEvent(EndFrameEvent&&) noexcept = default;
  EndFrameEvent& operator=(const EndFrameEvent&) = default;
  EndFrameEvent& operator=(EndFrameEvent&&) noexcept = default;
  virtual ~EndFrameEvent() = default;

  stringid::StringId GetType() const noexcept override;
};
}  // namespace frame
}  // namespace comet

#endif  // COMET_COMET_CORE_FRAME_EVENT_H_
