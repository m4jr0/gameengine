// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTRY_POINT_H_
#define COMET_COMET_ENTRY_POINT_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"
#include "comet/engine/engine.h"

extern comet::memory::UniquePtr<comet::Engine> comet::GenerateEngine();

namespace comet {}  // namespace comet

#endif  // COMET_COMET_ENTRY_POINT_H_
