// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ENTRY_POINT_H_
#define COMET_COMET_CORE_ENTRY_POINT_H_

#include "comet/core/engine.h"
#include "comet_precompile.h"

extern std::unique_ptr<comet::core::Engine> comet::core::CreateEngine();

namespace comet {}  // namespace comet

#endif  // COMET_COMET_CORE_ENTRY_POINT_H_
