// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_UTILS_LOGGER_HPP_
#define KOMA_UTILS_LOGGER_HPP_

#include <iostream>
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
    this->GetString(string_stream, args...);
    std::cerr << "[Error] " << string_stream.str() << std::endl;
  }

  template <typename... Targs>
  void Message(Targs... args) const {
    std::stringstream string_stream;
    this->GetString(string_stream, args...);
    std::cout << "[Message] " << string_stream.str() << std::endl;
  }

  template <typename... Targs>
  void Warning(Targs... args) const {
    std::stringstream string_stream;
    this->GetString(string_stream, args...);
    std::cout << "[Warning] " << string_stream.str() << std::endl;
  }

  const std::string name() const { return this->name_; };

 private:
  template <typename T>
  void GetString(std::stringstream& string_stream, T arg) const {
    string_stream << arg;
  }

  template <typename T, typename... Targs>
  void GetString(std::stringstream& string_stream, T arg, Targs... args)
    const {
    this->GetString(string_stream, arg);
    this->GetString(string_stream, args...);
  }

  static std::unordered_map<std::string, std::shared_ptr<const Logger>>
    loggers_;

  static std::shared_ptr<const Logger> Create(std::string);

  std::string name_;
};
};  // namespace koma

#endif  // KOMA_UTILS_LOGGER_HPP_