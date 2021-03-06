// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "logger.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace core {
Logger& Logger::Get(LoggerType logger_type) {
  const auto found = Logger::loggers_.find(logger_type);

  // If the logger does not exist, we create it.
  if (found == Logger::loggers_.end()) {
    return *Logger::loggers_.insert({logger_type, Logger::Create(logger_type)})
                .first->second;
  }

  return *found->second;
}

Logger::Logger(LoggerType logger_type) { type_ = logger_type; }

std::shared_ptr<Logger> Logger::Create(LoggerType logger_type) {
  // Because the constructor is private, we have to separate the initialization
  // from the shared pointer construction.
  // It should be safe though, as this line does nothing else than initializing
  // a logger instance and saving it to a shared pointer.
  // Performance-wise, it should be acceptable, as very few loggers will be
  // instanciated anyway.
  return std::shared_ptr<Logger>(new Logger(logger_type));
}

std::unordered_map<LoggerType, std::shared_ptr<Logger>> Logger::loggers_;
}  // namespace core
}  // namespace comet
