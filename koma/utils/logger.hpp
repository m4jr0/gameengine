// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_UTILS_LOGGER_HPP_
#define KOMA_UTILS_LOGGER_HPP_

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

namespace koma {
class Logger final {
 public:
  static std::shared_ptr<const Logger> Get(std::string);

  Logger() = delete;
  Logger(std::string);
  virtual ~Logger();

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

  static std::unordered_map<std::string, std::shared_ptr<const Logger>>
      loggers_;

  static std::shared_ptr<const Logger> Create(std::string);

  std::string name_;
};
}  // namespace koma

#endif  // KOMA_UTILS_LOGGER_HPP_
