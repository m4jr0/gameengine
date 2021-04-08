// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "logger.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
Logger::Logger(LoggerType logger_type) { type_ = logger_type; }

std::shared_ptr<const Logger> Logger::Get(LoggerType logger_type) {
  const auto found = Logger::loggers_.find(logger_type);

  // If the logger does not exist, we create it.
  if (found == Logger::loggers_.end()) {
    const auto new_logger = Logger::Create(logger_type);
    Logger::loggers_.insert({logger_type, new_logger});

    return new_logger;
  }

  return found->second;
}

std::shared_ptr<const Logger> Logger::Create(LoggerType logger_type) {
  return std::make_shared<const Logger>(logger_type);
}

std::unordered_map<LoggerType, std::shared_ptr<const Logger>> Logger::loggers_;
}  // namespace comet
