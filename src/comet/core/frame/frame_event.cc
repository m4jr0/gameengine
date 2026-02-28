// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "frame_event.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace frame {
const stringid::StringId NewFrameEvent::kStaticType_{
    COMET_STRING_ID("event_new_frame")};

stringid::StringId NewFrameEvent::GetType() const noexcept {
  return kStaticType_;
}

const stringid::StringId EndFrameEvent::kStaticType_{
    COMET_STRING_ID("event_end_frame")};

stringid::StringId EndFrameEvent::GetType() const noexcept {
  return kStaticType_;
}
}  // namespace frame
}  // namespace comet
