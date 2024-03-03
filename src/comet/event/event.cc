// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "event.h"

namespace comet {
namespace event {
Event::Event() : sequence_number_{sequence_number_count_++} {}

SequenceNumber Event::GetSequenceNumber() const noexcept {
  return sequence_number_;
}
}  // namespace event
}  // namespace comet
