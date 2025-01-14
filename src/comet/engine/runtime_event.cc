// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "runtime_event.h"

namespace comet {
const stringid::StringId UnrecoverableErrorEvent::kStaticType_{
    COMET_STRING_ID("event_unrecoverable_error")};

stringid::StringId UnrecoverableErrorEvent::GetType() const noexcept {
  return kStaticType_;
}
}  // namespace comet
