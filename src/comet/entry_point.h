// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTRY_POINT_H_
#define COMET_COMET_ENTRY_POINT_H_

#include "comet_precompile.h"

#include "comet/core/engine.h"

extern std::unique_ptr<comet::Engine> comet::GenerateEngine();

namespace comet {}  // namespace comet

#endif  // COMET_COMET_ENTRY_POINT_H_
