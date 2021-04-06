// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_UTILS_LOGGER_HPP_
#define COMET_UTILS_LOGGER_HPP_

#include "comet_precompile.hpp"

namespace comet {
class Logger final {
 public:
  static std::shared_ptr<const Logger> Get(std::string);

  Logger() = delete;
  Logger(std::string);

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

  const std::string name() const { return name_; };

 private:
  template <typename T>
  void GetString(std::stringstream& string_stream, T arg) const {
    string_stream << arg;
  }

  template <typename T, typename... Targs>
  void GetString(std::stringstream& string_stream, T arg, Targs... args) const {
    GetString(string_stream, arg);
    GetString(string_stream, args...);
  }

  static std::shared_ptr<const Logger> Create(std::string);

  static std::unordered_map<std::string, std::shared_ptr<const Logger>>
      loggers_;

  std::string name_;
};
}  // namespace comet

#endif  // COMET_UTILS_LOGGER_HPP_
