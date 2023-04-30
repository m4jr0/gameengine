// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "debug.h"

#include "comet/core/logger.h"

namespace comet {
// TODO(m4jr0): Handle critical error properly.
void HandleCriticalError() { COMET_LOG_GLOBAL_ERROR("Aborting."); }
}  // namespace comet
