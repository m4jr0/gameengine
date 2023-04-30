// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_LOGGER_H_
#define COMET_COMET_CORE_LOGGER_H_

#include "comet_precompile.h"

// If issues arise with current terminal, comment this line.
#define COMET_TERMINAL_COLORS

#define COMET_ASCII_PREFIX "\033["
#define COMET_ASCII_SUFFIX "m"
#define COMET_ASCII_ATTR_SUFFIX ";"
#define COMET_ASCII_EMPHASIS_ATTR "1" COMET_ASCII_ATTR_SUFFIX

#define COMET_ASCII_NORMAL_COL "0"  // Default terminal color.
#define COMET_ASCII_INFO_COL COMET_ASCII_NORMAL_COL
#define COMET_ASCII_ERROR_COL "31"    // Red color.
#define COMET_ASCII_WARNING_COL "33"  // Yellow color.
#define COMET_ASCII_DEBUG_COL "90"    // Dark gray color.

#ifdef COMET_TERMINAL_COLORS
#define COMET_ASCII_CATEGORY(COLOR) \
  COMET_ASCII_PREFIX COMET_ASCII_EMPHASIS_ATTR COLOR COMET_ASCII_SUFFIX
#define COMET_ASCII(COLOR) COMET_ASCII_PREFIX COLOR COMET_ASCII_SUFFIX
#define COMET_ASCII_RESET COMET_ASCII(COMET_ASCII_NORMAL_COL)
#else
#define COMET_ASCII_CATEGORY(COLOR) ""
#define COMET_ASCII(COLOR) ""
#define COMET_ASCII_RESET ""
#endif  // COMET_TERMINAL_COLORS

namespace comet {
enum class LoggerType {
  Unknown = 0,
  Global,
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
  void Error(const Targs&... args) const {
    std::stringstream string_stream;
    GetString(string_stream, args...);
    std::cerr << COMET_ASCII_CATEGORY(COMET_ASCII_ERROR_COL) << "[ERROR]"
              << COMET_ASCII(COMET_ASCII_ERROR_COL) << " "
              << string_stream.str() << COMET_ASCII_RESET << '\n';
  }

  template <typename... Targs>
  void Info(const Targs&... args) const {
    std::stringstream string_stream;
    GetString(string_stream, args...);
    std::cout << COMET_ASCII_CATEGORY(COMET_ASCII_INFO_COL) << "[INFO]"
              << COMET_ASCII(COMET_ASCII_INFO_COL) << " " << string_stream.str()
              << COMET_ASCII_RESET << '\n';
  }

  template <typename... Targs>
  void Debug(const Targs&... args) const {
    std::stringstream string_stream;
    GetString(string_stream, args...);
    std::cout << COMET_ASCII_CATEGORY(COMET_ASCII_DEBUG_COL) << "[DEBUG]"
              << COMET_ASCII(COMET_ASCII_DEBUG_COL) << " "
              << string_stream.str() << COMET_ASCII_RESET << '\n';
  }

  template <typename... Targs>
  void Warning(const Targs&... args) const {
    std::stringstream string_stream;
    GetString(string_stream, args...);
    std::cout << COMET_ASCII_CATEGORY(COMET_ASCII_WARNING_COL) << "[WARNING]"
              << COMET_ASCII(COMET_ASCII_WARNING_COL) << " "
              << string_stream.str() << COMET_ASCII_RESET << '\n';
  }

  const LoggerType GetType() const { return type_; };

 private:
  explicit Logger(LoggerType logger_type);

  template <typename T>
  void GetString(std::stringstream& string_stream, const T& arg) const {
    string_stream << arg;
  }

  template <typename T, typename... Targs>
  void GetString(std::stringstream& string_stream, const T& arg,
                 const Targs&... args) const {
    GetString(string_stream, arg);
    GetString(string_stream, args...);
  }

  static std::shared_ptr<Logger> Generate(LoggerType logger_type);

  static std::unordered_map<LoggerType, std::shared_ptr<Logger>> loggers_;
  LoggerType type_{LoggerType::Unknown};
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
  comet::Logger::Get(logger_type).Error(__VA_ARGS__)
#define COMET_LOG_INFO(logger_type, ...) \
  comet::Logger::Get(logger_type).Info(__VA_ARGS__)
#define COMET_LOG_DEBUG(logger_type, ...) \
  comet::Logger::Get(logger_type).Debug(__VA_ARGS__)
#define COMET_LOG_WARNING(logger_type, ...) \
  comet::Logger::Get(logger_type).Warning(__VA_ARGS__)

#define COMET_LOG_GLOBAL_ERROR(...) \
  comet::Logger::Get(comet::LoggerType::Global).Error(__VA_ARGS__)
#define COMET_LOG_GLOBAL_INFO(...) \
  comet::Logger::Get(comet::LoggerType::Global).Info(__VA_ARGS__)
#define COMET_LOG_GLOBAL_DEBUG(...) \
  comet::Logger::Get(comet::LoggerType::Global).Debug(__VA_ARGS__)
#define COMET_LOG_GLOBAL_WARNING(...) \
  comet::Logger::Get(comet::LoggerType::Global).Warning(__VA_ARGS__)

#define COMET_LOG_CORE_ERROR(...) \
  comet::Logger::Get(comet::LoggerType::Core).Error(__VA_ARGS__)
#define COMET_LOG_CORE_INFO(...) \
  comet::Logger::Get(comet::LoggerType::Core).Info(__VA_ARGS__)
#define COMET_LOG_CORE_DEBUG(...) \
  comet::Logger::Get(comet::LoggerType::Core).Debug(__VA_ARGS__)
#define COMET_LOG_CORE_WARNING(...) \
  comet::Logger::Get(comet::LoggerType::Core).Warning(__VA_ARGS__)

#define COMET_LOG_EVENT_ERROR(...) \
  comet::Logger::Get(comet::LoggerType::Event).Error(__VA_ARGS__)
#define COMET_LOG_EVENT_INFO(...) \
  comet::Logger::Get(comet::LoggerType::Event).Info(__VA_ARGS__)
#define COMET_LOG_EVENT_DEBUG(...) \
  comet::Logger::Get(comet::LoggerType::Event).Debug(__VA_ARGS__)
#define COMET_LOG_EVENT_WARNING(...) \
  comet::Logger::Get(comet::LoggerType::Event).Warning(__VA_ARGS__)

#define COMET_LOG_ENTITY_ERROR(...) \
  comet::Logger::Get(comet::LoggerType::GameObject).Error(__VA_ARGS__)
#define COMET_LOG_ENTITY_INFO(...) \
  comet::Logger::Get(comet::LoggerType::GameObject).Info(__VA_ARGS__)
#define COMET_LOG_ENTITY_DEBUG(...) \
  comet::Logger::Get(comet::LoggerType::GameObject).Debug(__VA_ARGS__)
#define COMET_LOG_ENTITY_WARNING(...) \
  comet::Logger::Get(comet::LoggerType::GameObject).Warning(__VA_ARGS__)

#define COMET_LOG_INPUT_ERROR(...) \
  comet::Logger::Get(comet::LoggerType::Input).Error(__VA_ARGS__)
#define COMET_LOG_INPUT_INFO(...) \
  comet::Logger::Get(comet::LoggerType::Input).Info(__VA_ARGS__)
#define COMET_LOG_INPUT_DEBUG(...) \
  comet::Logger::Get(comet::LoggerType::Input).Debug(__VA_ARGS__)
#define COMET_LOG_INPUT_WARNING(...) \
  comet::Logger::Get(comet::LoggerType::Input).Warning(__VA_ARGS__)

#define COMET_LOG_PHYSICS_ERROR(...) \
  comet::Logger::Get(comet::LoggerType::Physics).Error(__VA_ARGS__)
#define COMET_LOG_PHYSICS_INFO(...) \
  comet::Logger::Get(comet::LoggerType::Physics).Info(__VA_ARGS__)
#define COMET_LOG_PHYSICS_DEBUG(...) \
  comet::Logger::Get(comet::LoggerType::Physics).Debug(__VA_ARGS__)
#define COMET_LOG_PHYSICS_WARNING(...) \
  comet::Logger::Get(comet::LoggerType::Physics).Warning(__VA_ARGS__)

#define COMET_LOG_RENDERING_ERROR(...) \
  comet::Logger::Get(comet::LoggerType::Rendering).Error(__VA_ARGS__)
#define COMET_LOG_RENDERING_INFO(...) \
  comet::Logger::Get(comet::LoggerType::Rendering).Info(__VA_ARGS__)
#define COMET_LOG_RENDERING_DEBUG(...) \
  comet::Logger::Get(comet::LoggerType::Rendering).Debug(__VA_ARGS__)
#define COMET_LOG_RENDERING_WARNING(...) \
  comet::Logger::Get(comet::LoggerType::Rendering).Warning(__VA_ARGS__)

#define COMET_LOG_RESOURCE_ERROR(...) \
  comet::Logger::Get(comet::LoggerType::Resource).Error(__VA_ARGS__)
#define COMET_LOG_RESOURCE_INFO(...) \
  comet::Logger::Get(comet::LoggerType::Resource).Info(__VA_ARGS__)
#define COMET_LOG_RESOURCE_DEBUG(...) \
  comet::Logger::Get(comet::LoggerType::Resource).Debug(__VA_ARGS__)
#define COMET_LOG_RESOURCE_WARNING(...) \
  comet::Logger::Get(comet::LoggerType::Resource).Warning(__VA_ARGS__)

#define COMET_LOG_TIME_ERROR(...) \
  comet::Logger::Get(comet::LoggerType::Time).Error(__VA_ARGS__)
#define COMET_LOG_TIME_INFO(...) \
  comet::Logger::Get(comet::LoggerType::Time).Info(__VA_ARGS__)
#define COMET_LOG_TIME_DEBUG(...) \
  comet::Logger::Get(comet::LoggerType::Time).Debug(__VA_ARGS__)
#define COMET_LOG_TIME_WARNING(...) \
  comet::Logger::Get(comet::LoggerType::Time).Warning(__VA_ARGS__)

#define COMET_LOG_UTILS_ERROR(...) \
  comet::Logger::Get(comet::LoggerType::Utils).Error(__VA_ARGS__)
#define COMET_LOG_UTILS_INFO(...) \
  comet::Logger::Get(comet::LoggerType::Utils).Info(__VA_ARGS__)
#define COMET_LOG_UTILS_DEBUG(...) \
  comet::Logger::Get(comet::LoggerType::Utils).Debug(__VA_ARGS__)
#define COMET_LOG_UTILS_WARNING(...) \
  comet::Logger::Get(comet::LoggerType::Utils).Warning(__VA_ARGS__)
#endif  // !COMET_DEBUG
}  // namespace comet

#endif  // COMET_COMET_CORE_LOGGER_H_
