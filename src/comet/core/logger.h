// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_LOGGER_H_
#define COMET_COMET_CORE_LOGGER_H_

#include "comet_precompile.h"

namespace comet {
namespace log {
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
  ~Logger() = default;

  template <typename... Targs>
  void Error(Targs&&... args) const {
    std::stringstream string_stream;
    GetString(string_stream, std::forward<Targs>(args)...);
    std::cerr << "[ERROR] " << string_stream.str() << '\n';
  }

  template <typename... Targs>
  void Info(Targs&&... args) const {
    std::stringstream string_stream;
    GetString(string_stream, std::forward<Targs>(args)...);
    std::cout << "[INFO] " << string_stream.str() << '\n';
  }

  template <typename... Targs>
  void Debug(Targs&&... args) const {
    std::stringstream string_stream;
    GetString(string_stream, std::forward<Targs>(args)...);
    std::cout << "[DEBUG] " << string_stream.str() << '\n';
  }

  template <typename... Targs>
  void Warning(Targs&&... args) const {
    std::stringstream string_stream;
    GetString(string_stream, std::forward<Targs>(args)...);
    std::cout << "[WARNING] " << string_stream.str() << '\n';
  }

  const LoggerType GetType() const { return type_; };

 private:
  Logger(LoggerType);

  template <typename T>
  void GetString(std::stringstream& string_stream, T&& arg) const {
    string_stream << std::forward<T>(arg);
  }

  template <typename T, typename... Targs>
  void GetString(std::stringstream& string_stream, T&& arg,
                 Targs&&... args) const {
    GetString(string_stream, std::forward<T>(arg));
    GetString(string_stream, std::forward<Targs>(args)...);
  }

  static std::shared_ptr<Logger> Create(LoggerType logger_type);

  static std::unordered_map<LoggerType, std::shared_ptr<Logger>> loggers_;

  LoggerType type_;
};

#ifndef COMET_DEBUG
#define COMET_LOG_ERROR(logger_type, ...)
#define COMET_LOG_INFO(logger_type, ...)
#define COMET_LOG_DEBUG(logger_type, ...)
#define COMET_LOG_WARNING(logger_type, ...)

#define COMET_LOG_GLOBAL_ERROR(...)
#define COMET_LOG_GLOBAL_INFO(...)
#define COMET_LOG_GLOBAL_DEBUG(...)
#define COMET_LOG_GLOBAL_WARNING(...)

#define COMET_LOG_CORE_ERROR(...)
#define COMET_LOG_CORE_INFO(...)
#define COMET_LOG_CORE_DEBUG(...)
#define COMET_LOG_CORE_WARNING(...)

#define COMET_LOG_EVENT_ERROR(...)
#define COMET_LOG_EVENT_INFO(...)
#define COMET_LOG_EVENT_DEBUG(...)
#define COMET_LOG_EVENT_WARNING(...)

#define COMET_LOG_ENTITY_ERROR(...)
#define COMET_LOG_ENTITY_INFO(...)
#define COMET_LOG_ENTITY_DEBUG(...)
#define COMET_LOG_ENTITY_WARNING(...)

#define COMET_LOG_INPUT_ERROR(...)
#define COMET_LOG_INPUT_INFO(...)
#define COMET_LOG_INPUT_DEBUG(...)
#define COMET_LOG_INPUT_WARNING(...)

#define COMET_LOG_PHYSICS_ERROR(...)
#define COMET_LOG_PHYSICS_INFO(...)
#define COMET_LOG_PHYSICS_DEBUG(...)
#define COMET_LOG_PHYSICS_WARNING(...)

#define COMET_LOG_RENDERING_ERROR(...)
#define COMET_LOG_RENDERING_INFO(...)
#define COMET_LOG_RENDERING_DEBUG(...)
#define COMET_LOG_RENDERING_WARNING(...)

#define COMET_LOG_RESOURCE_ERROR(...)
#define COMET_LOG_RESOURCE_INFO(...)
#define COMET_LOG_RESOURCE_DEBUG(...)
#define COMET_LOG_RESOURCE_WARNING(...)

#define COMET_LOG_TIME_ERROR(...)
#define COMET_LOG_TIME_INFO(...)
#define COMET_LOG_TIME_DEBUG(...)
#define COMET_LOG_TIME_WARNING(...)

#define COMET_LOG_UTILS_ERROR(...)
#define COMET_LOG_UTILS_INFO(...)
#define COMET_LOG_UTILS_DEBUG(...)
#define COMET_LOG_UTILS_WARNING(...)
#else
#define COMET_LOG_ERROR(logger_type, ...) \
  comet::log::Logger::Get(logger_type).Error(__VA_ARGS__)
#define COMET_LOG_INFO(logger_type, ...) \
  comet::log::Logger::Get(logger_type).Info(__VA_ARGS__)
#define COMET_LOG_DEBUG(logger_type, ...) \
  comet::log::Logger::Get(logger_type).Debug(__VA_ARGS__)
#define COMET_LOG_WARNING(logger_type, ...) \
  comet::log::Logger::Get(logger_type).Warning(__VA_ARGS__)

#define COMET_LOG_GLOBAL_ERROR(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Global).Error(__VA_ARGS__)
#define COMET_LOG_GLOBAL_INFO(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Global).Info(__VA_ARGS__)
#define COMET_LOG_GLOBAL_DEBUG(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Global).Debug(__VA_ARGS__)
#define COMET_LOG_GLOBAL_WARNING(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Global).Warning(__VA_ARGS__)

#define COMET_LOG_CORE_ERROR(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Core).Error(__VA_ARGS__)
#define COMET_LOG_CORE_INFO(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Core).Info(__VA_ARGS__)
#define COMET_LOG_CORE_DEBUG(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Core).Debug(__VA_ARGS__)
#define COMET_LOG_CORE_WARNING(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Core).Warning(__VA_ARGS__)

#define COMET_LOG_EVENT_ERROR(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Event).Error(__VA_ARGS__)
#define COMET_LOG_EVENT_INFO(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Event).Info(__VA_ARGS__)
#define COMET_LOG_EVENT_DEBUG(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Event).Debug(__VA_ARGS__)
#define COMET_LOG_EVENT_WARNING(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Event).Warning(__VA_ARGS__)

#define COMET_LOG_ENTITY_ERROR(...) \
  comet::log::Logger::Get(comet::log::LoggerType::GameObject).Error(__VA_ARGS__)
#define COMET_LOG_ENTITY_INFO(...) \
  comet::log::Logger::Get(comet::log::LoggerType::GameObject).Info(__VA_ARGS__)
#define COMET_LOG_ENTITY_DEBUG(...) \
  comet::log::Logger::Get(comet::log::LoggerType::GameObject).Debug(__VA_ARGS__)
#define COMET_LOG_ENTITY_WARNING(...)                         \
  comet::log::Logger::Get(comet::log::LoggerType::GameObject) \
      .Warning(__VA_ARGS__)

#define COMET_LOG_INPUT_ERROR(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Input).Error(__VA_ARGS__)
#define COMET_LOG_INPUT_INFO(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Input).Info(__VA_ARGS__)
#define COMET_LOG_INPUT_DEBUG(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Input).Debug(__VA_ARGS__)
#define COMET_LOG_INPUT_WARNING(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Input).Warning(__VA_ARGS__)

#define COMET_LOG_PHYSICS_ERROR(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Physics).Error(__VA_ARGS__)
#define COMET_LOG_PHYSICS_INFO(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Physics).Info(__VA_ARGS__)
#define COMET_LOG_PHYSICS_DEBUG(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Physics).Debug(__VA_ARGS__)
#define COMET_LOG_PHYSICS_WARNING(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Physics).Warning(__VA_ARGS__)

#define COMET_LOG_RENDERING_ERROR(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Rendering).Error(__VA_ARGS__)
#define COMET_LOG_RENDERING_INFO(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Rendering).Info(__VA_ARGS__)
#define COMET_LOG_RENDERING_DEBUG(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Rendering).Debug(__VA_ARGS__)
#define COMET_LOG_RENDERING_WARNING(...)                     \
  comet::log::Logger::Get(comet::log::LoggerType::Rendering) \
      .Warning(__VA_ARGS__)

#define COMET_LOG_RESOURCE_ERROR(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Resource).Error(__VA_ARGS__)
#define COMET_LOG_RESOURCE_INFO(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Resource).Info(__VA_ARGS__)
#define COMET_LOG_RESOURCE_DEBUG(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Resource).Debug(__VA_ARGS__)
#define COMET_LOG_RESOURCE_WARNING(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Resource).Warning(__VA_ARGS__)

#define COMET_LOG_TIME_ERROR(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Time).Error(__VA_ARGS__)
#define COMET_LOG_TIME_INFO(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Time).Info(__VA_ARGS__)
#define COMET_LOG_TIME_DEBUG(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Time).Debug(__VA_ARGS__)
#define COMET_LOG_TIME_WARNING(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Time).Warning(__VA_ARGS__)

#define COMET_LOG_UTILS_ERROR(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Utils).Error(__VA_ARGS__)
#define COMET_LOG_UTILS_INFO(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Utils).Info(__VA_ARGS__)
#define COMET_LOG_UTILS_DEBUG(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Utils).Debug(__VA_ARGS__)
#define COMET_LOG_UTILS_WARNING(...) \
  comet::log::Logger::Get(comet::log::LoggerType::Utils).Warning(__VA_ARGS__)
#endif  // !COMET_DEBUG
}  // namespace log
}  // namespace comet

#endif  // COMET_COMET_CORE_LOGGER_H_
