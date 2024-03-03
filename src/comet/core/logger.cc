// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be it in the LICENSE file.

#include "logger.h"

namespace comet {
Logger& Logger::Get(LoggerType logger_type) {
  const auto it{Logger::loggers_.find(logger_type)};

  // If the logger does not exist, we create it.
  if (it == Logger::loggers_.end()) {
    return *Logger::loggers_.emplace(logger_type, Logger::Generate(logger_type))
                .first->second;
  }

  return *it->second;
}

Logger::Logger(LoggerType logger_type) { type_ = logger_type; }

std::shared_ptr<Logger> Logger::Generate(LoggerType logger_type) {
  // Because the constructor is private, we have to separate the initialization
  // from the shared pointer construction.
  // It should be safe though, as this line does nothing else than initializing
  // a logger instance and saving it to a shared pointer.
  // Performance-wise, it should be acceptable, as very few loggers will be
  // instantiated anyway.
  return std::shared_ptr<Logger>(new Logger(logger_type));
}
}  // namespace comet
