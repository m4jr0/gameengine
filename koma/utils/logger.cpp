// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "logger.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

namespace koma {
Logger::Logger(std::string logger_name) { name_ = logger_name; }

std::shared_ptr<const Logger> Logger::Get(std::string logger_name) {
  const auto found = Logger::loggers_.find(logger_name);

  // If the logger does not exist, we create it.
  if (found == Logger::loggers_.end()) {
    const auto new_logger = Logger::Create(logger_name);
    Logger::loggers_.insert({logger_name, new_logger});

    return new_logger;
  }

  return found->second;
}

std::shared_ptr<const Logger> Logger::Create(std::string logger_name) {
  return std::make_shared<const Logger>(logger_name);
}

std::unordered_map<std::string, std::shared_ptr<const Logger>> Logger::loggers_;
}  // namespace koma
