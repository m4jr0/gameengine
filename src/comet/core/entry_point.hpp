// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_ENTRY_POINT_HPP_
#define COMET_CORE_ENTRY_POINT_HPP_

#include "comet_precompile.hpp"
#include "engine.hpp"

extern std::unique_ptr<comet::Engine> comet::CreateEngine();

namespace comet {}  // namespace comet

#endif  // COMET_CORE_ENTRY_POINT_HPP_
