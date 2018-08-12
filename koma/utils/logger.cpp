// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allows debugging memory leaks.
// TODO(m4jr0): Find a better solution.
#include "../debug.hpp"

#include "logger.hpp"

namespace koma {
std::unordered_map<std::string, std::shared_ptr<const Logger>>
  Logger::loggers_;

std::shared_ptr<const Logger> Logger::Get(std::string logger_name) {
  auto found = Logger::loggers_.find(logger_name);

  // If the logger does not exist, we create it.
  if (found == Logger::loggers_.end()) {
    std::shared_ptr<const Logger> new_logger = Logger::Create(logger_name);
    Logger::loggers_.insert({logger_name, new_logger});

    return new_logger;
  }

  return found->second;
}

std::shared_ptr<const Logger> Logger::Create(std::string logger_name) {
  return std::make_shared<const Logger>(logger_name);
}

Logger::Logger(std::string logger_name) { this->name_ = logger_name; }

Logger::~Logger() {}
};  // namespace koma
