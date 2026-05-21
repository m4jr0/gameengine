// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be it in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/logger/logger_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
const schar* GetLoggerTypeLabel(LoggerType type) {
  switch (type) {
    case LoggerType::Animation:
      return "animation";
    case LoggerType::Core:
      return "core";
    case LoggerType::Event:
      return "event";
    case LoggerType::Engine:
      return "engine";
    case LoggerType::Entity:
      return "entity";
    case LoggerType::Geometry:
      return "geometry";
    case LoggerType::Input:
      return "input";
    case LoggerType::Math:
      return "math";
    case LoggerType::Physics:
      return "physics";
    case LoggerType::Profiler:
      return "profiler";
    case LoggerType::Rendering:
      return "rendering";
    case LoggerType::Resource:
      return "resource";
    case LoggerType::Time:
      return "time";
    case LoggerType::External:
      return "external";
    default:
      return kUnknownLabel;
  }
}

const schar* GetLogLevelLabel(LogLevel level) {
  switch (level) {
    case LogLevel::Info:
      return "info";
    case LogLevel::Warning:
      return "warning";
    case LogLevel::Error:
      return "error";
    case LogLevel::Debug:
      return "debug";
    default:
      return kUnknownLabel;
  }
}
}  // namespace comet
