// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_LOGGER_H_
#define COMET_COMET_CORE_LOGGER_H_

#include "comet_precompile.h"

namespace comet {
namespace core {
enum class LoggerType {
  Global = 0,
  Core,
  Event,
  GameObject,
  Input,
  Physics,
  Rendering,
  Resource,
  Time,
  Utils
};

class Logger final {
 public:
  static Logger& Get(LoggerType);

  Logger() = delete;
  Logger(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&) = delete;
  virtual ~Logger() = default;

  template <typename... Targs>
  void Error(Targs... args) const {
    std::stringstream string_stream;
    GetString(string_stream, args...);
    std::cerr << "[ERROR] " << string_stream.str() << std::endl;
  }

  template <typename... Targs>
  void Info(Targs... args) const {
    std::stringstream string_stream;
    GetString(string_stream, args...);
    std::cout << "[INFO] " << string_stream.str() << std::endl;
  }

  template <typename... Targs>
  void Debug(Targs... args) const {
    std::stringstream string_stream;
    GetString(string_stream, args...);
    std::cout << "[DEBUG] " << string_stream.str() << std::endl;
  }

  template <typename... Targs>
  void Warning(Targs... args) const {
    std::stringstream string_stream;
    GetString(string_stream, args...);
    std::cout << "[WARNING] " << string_stream.str() << std::endl;
  }

  const LoggerType GetType() const { return type_; };

 private:
  Logger(LoggerType);

  template <typename T>
  void GetString(std::stringstream& string_stream, T arg) const {
    string_stream << arg;
  }

  template <typename T, typename... Targs>
  void GetString(std::stringstream& string_stream, T arg, Targs... args) const {
    GetString(string_stream, arg);
    GetString(string_stream, args...);
  }

  static std::shared_ptr<Logger> Create(LoggerType);

  static std::unordered_map<LoggerType, std::shared_ptr<Logger>> loggers_;

  LoggerType type_;
};
}  // namespace core
}  // namespace comet

#endif  // COMET_COMET_CORE_LOGGER_H_
