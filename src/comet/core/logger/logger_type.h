// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_LOGGER_LOGGER_TYPE_H_
#define COMET_COMET_CORE_LOGGER_LOGGER_TYPE_H_

#include "comet/core/essentials.h"

namespace comet {
enum class LoggerType {
  Unknown = 0,
  Animation,
  Core,
  Event,
  Engine,
  Entity,
  Geometry,
  Input,
  Math,
  Physics,
  Profiler,
  Rendering,
  Resource,
  Time,
  External
};

enum class LogLevel { Info = 0, Warning, Error, Debug };
}  // namespace comet

#endif  // COMET_COMET_CORE_LOGGER_LOGGER_TYPE_H_