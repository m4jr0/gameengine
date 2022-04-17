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

  static std::shared_ptr<Logger> Create(LoggerType logger_type);

  static std::unordered_map<LoggerType, std::shared_ptr<Logger>> loggers_;

  LoggerType type_;
};

#ifdef NDEBUG
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

#define COMET_LOG_GAME_OBJECT_ERROR(...)
#define COMET_LOG_GAME_OBJECT_INFO(...)
#define COMET_LOG_GAME_OBJECT_DEBUG(...)
#define COMET_LOG_GAME_OBJECT_WARNING(...)

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
  comet::core::Logger::Get(logger_type).Error(__VA_ARGS__)
#define COMET_LOG_INFO(logger_type, ...) \
  comet::core::Logger::Get(logger_type).Info(__VA_ARGS__)
#define COMET_LOG_DEBUG(logger_type, ...) \
  comet::core::Logger::Get(logger_type).Debug(__VA_ARGS__)
#define COMET_LOG_WARNING(logger_type, ...) \
  comet::core::Logger::Get(logger_type).Warning(__VA_ARGS__)

#define COMET_LOG_GLOBAL_ERROR(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Global).Error(__VA_ARGS__)
#define COMET_LOG_GLOBAL_INFO(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Global).Info(__VA_ARGS__)
#define COMET_LOG_GLOBAL_DEBUG(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Global).Debug(__VA_ARGS__)
#define COMET_LOG_GLOBAL_WARNING(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Global).Warning(__VA_ARGS__)

#define COMET_LOG_CORE_ERROR(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Core).Error(__VA_ARGS__)
#define COMET_LOG_CORE_INFO(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Core).Info(__VA_ARGS__)
#define COMET_LOG_CORE_DEBUG(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Core).Debug(__VA_ARGS__)
#define COMET_LOG_CORE_WARNING(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Core).Warning(__VA_ARGS__)

#define COMET_LOG_EVENT_ERROR(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Event).Error(__VA_ARGS__)
#define COMET_LOG_EVENT_INFO(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Event).Info(__VA_ARGS__)
#define COMET_LOG_EVENT_DEBUG(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Event).Debug(__VA_ARGS__)
#define COMET_LOG_EVENT_WARNING(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Event).Warning(__VA_ARGS__)

#define COMET_LOG_GAME_OBJECT_ERROR(...)                        \
  comet::core::Logger::Get(comet::core::LoggerType::GameObject) \
      .Error(__VA_ARGS__)
#define COMET_LOG_GAME_OBJECT_INFO(...)                         \
  comet::core::Logger::Get(comet::core::LoggerType::GameObject) \
      .Info(__VA_ARGS__)
#define COMET_LOG_GAME_OBJECT_DEBUG(...)                        \
  comet::core::Logger::Get(comet::core::LoggerType::GameObject) \
      .Debug(__VA_ARGS__)
#define COMET_LOG_GAME_OBJECT_WARNING(...)                      \
  comet::core::Logger::Get(comet::core::LoggerType::GameObject) \
      .Warning(__VA_ARGS__)

#define COMET_LOG_INPUT_ERROR(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Input).Error(__VA_ARGS__)
#define COMET_LOG_INPUT_INFO(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Input).Info(__VA_ARGS__)
#define COMET_LOG_INPUT_DEBUG(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Input).Debug(__VA_ARGS__)
#define COMET_LOG_INPUT_WARNING(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Input).Warning(__VA_ARGS__)

#define COMET_LOG_PHYSICS_ERROR(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Physics).Error(__VA_ARGS__)
#define COMET_LOG_PHYSICS_INFO(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Physics).Info(__VA_ARGS__)
#define COMET_LOG_PHYSICS_DEBUG(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Physics).Debug(__VA_ARGS__)
#define COMET_LOG_PHYSICS_WARNING(...)                       \
  comet::core::Logger::Get(comet::core::LoggerType::Physics) \
      .Warning(__VA_ARGS__)

#define COMET_LOG_RENDERING_ERROR(...)                         \
  comet::core::Logger::Get(comet::core::LoggerType::Rendering) \
      .Error(__VA_ARGS__)
#define COMET_LOG_RENDERING_INFO(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Rendering).Info(__VA_ARGS__)
#define COMET_LOG_RENDERING_DEBUG(...)                         \
  comet::core::Logger::Get(comet::core::LoggerType::Rendering) \
      .Debug(__VA_ARGS__)
#define COMET_LOG_RENDERING_WARNING(...)                       \
  comet::core::Logger::Get(comet::core::LoggerType::Rendering) \
      .Warning(__VA_ARGS__)

#define COMET_LOG_RESOURCE_ERROR(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Resource).Error(__VA_ARGS__)
#define COMET_LOG_RESOURCE_INFO(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Resource).Info(__VA_ARGS__)
#define COMET_LOG_RESOURCE_DEBUG(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Resource).Debug(__VA_ARGS__)
#define COMET_LOG_RESOURCE_WARNING(...)                       \
  comet::core::Logger::Get(comet::core::LoggerType::Resource) \
      .Warning(__VA_ARGS__)

#define COMET_LOG_TIME_ERROR(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Time).Error(__VA_ARGS__)
#define COMET_LOG_TIME_INFO(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Time).Info(__VA_ARGS__)
#define COMET_LOG_TIME_DEBUG(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Time).Debug(__VA_ARGS__)
#define COMET_LOG_TIME_WARNING(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Time).Warning(__VA_ARGS__)

#define COMET_LOG_UTILS_ERROR(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Utils).Error(__VA_ARGS__)
#define COMET_LOG_UTILS_INFO(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Utils).Info(__VA_ARGS__)
#define COMET_LOG_UTILS_DEBUG(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Utils).Debug(__VA_ARGS__)
#define COMET_LOG_UTILS_WARNING(...) \
  comet::core::Logger::Get(comet::core::LoggerType::Utils).Warning(__VA_ARGS__)
#endif
}  // namespace core
}  // namespace comet

#endif  // COMET_COMET_CORE_LOGGER_H_
