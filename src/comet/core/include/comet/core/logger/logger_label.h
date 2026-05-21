// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_LOGGER_LOGGER_LABEL_H_
#define COMET_CORE_LOGGER_LOGGER_LABEL_H_

#include "comet/core/essentials.h"
#include "comet/core/logger/logger_type.h"

namespace comet {
const schar* GetLoggerTypeLabel(LoggerType type);
const schar* GetLogLevelLabel(LogLevel level);
}  // namespace comet

#endif  // COMET_CORE_LOGGER_LOGGER_LABEL_H_