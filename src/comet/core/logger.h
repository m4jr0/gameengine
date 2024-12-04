// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_LOGGER_H_
#define COMET_COMET_CORE_LOGGER_H_

#include <atomic>
#include <string_view>

#include "comet/core/concurrency/thread/thread.h"
#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/tstring.h"

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
  Animation,
  Core,
  Event,
  Entity,
  Input,
  Math,
  Physics,
  Profiler,
  Rendering,
  Resource,
  Time
};

const schar* GetLoggerTypeLabel(LoggerType type);

class Logger final {
  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> needs to be always lock-free. Unsupported "
                "architecture");
  static_assert(std::atomic<bool>::is_always_lock_free,
                "std::atomic<bool> needs to be always lock-free. Unsupported "
                "architecture");

 public:
  void Destroy();

 private:
  void AddToBuffer(schar* buffer, usize len, usize& offset, const TString& arg);
  void AddToBuffer(schar* buffer, usize len, usize& offset, const schar* arg);
  void AddToBuffer(schar* buffer, usize len, usize& offset, const wchar* arg);
  void AddToBuffer(schar* buffer, usize len, usize& offset, CTStringView arg);
  void AddToBuffer(schar* buffer, usize len, usize& offset,
                   std::string_view arg);
  void Send(const schar* buffer, usize buffer_len);

 public:
  template <typename T>
  typename std::enable_if<
      std::is_fundamental<T>::value && !is_char_pointer<T>::value, void>::type
  AddToBuffer(schar* buffer, usize buffer_len, usize& offset, const T& arg) {
    // TODO(m4jr0): Use buffer from allocator.
    constexpr auto kSize{512};
    schar tmp[kSize]{'\0'};

    usize out_len;
    ConvertToStr(arg, tmp, kSize, &out_len);
    AddToBuffer(buffer, buffer_len, offset, tmp);
  }

  template <typename... Targs>
  void ProcessArgs(schar* buffer, usize buffer_len, usize& offset,
                   Targs... args) {
    (AddToBuffer(buffer, buffer_len, offset, args), ...);
  }

  template <typename... Targs>
  void ProcessLog(schar* buffer, usize buffer_len, usize& offset,
                  Targs... args) {
    ProcessArgs(buffer, buffer_len, offset, args...);
  }

  template <typename... Targs>
  void ProcessLogLn(schar* buffer, usize buffer_len, usize& offset,
                    Targs... args) {
    ProcessLog(buffer, buffer_len, offset, args...);
    AddToBuffer(buffer, buffer_len, offset, std::string_view{"\n"});
  }

  static Logger& Get();

  Logger(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&) = delete;
  ~Logger();

  template <typename... Targs>
  void Log(const schar* category, const schar* label_prefix,
           const schar* type_label, const schar* label_suffix,
           const schar* color, const Targs&... args) {
    // TODO(m4jr0): Use buffer from allocator.
    constexpr auto kSize{512};
    schar tmp[kSize]{'\0'};
    usize offset{0};

#ifdef COMET_LOG_IS_FIBER_PREFIX
    constexpr auto kEngineMemoryTagFiberPrefixSize{256};
    schar fiber_prefix[kEngineMemoryTagFiberPrefixSize]{'\0'};
    PopulateFiberPrefix(fiber_prefix, kEngineMemoryTagFiberPrefixSize);
    this->ProcessLog(tmp, kSize, offset, fiber_prefix);
#endif  // COMET_LOG_IS_FIBER_PREFIX

    this->ProcessLog(tmp, kSize, offset, category, label_prefix, type_label,
                     label_suffix, color);
    this->ProcessLog(tmp, kSize, offset, args...);
    this->ProcessLogLn(tmp, kSize, offset, COMET_ASCII_RESET);
    this->Send(tmp, offset);
  }

  template <typename... Targs>
  void Error(LoggerType type, const Targs&... args) {
    Log(COMET_ASCII_CATEGORY(COMET_ASCII_ERROR_COL), "[",
        GetLoggerTypeLabel(type), "] [ERROR] ",
        COMET_ASCII(COMET_ASCII_ERROR_COL), args...);
  }

  template <typename... Targs>
  void Warning(LoggerType type, const Targs&... args) {
    Log(COMET_ASCII_CATEGORY(COMET_ASCII_WARNING_COL), "[",
        GetLoggerTypeLabel(type), "] [WARNING] ",
        COMET_ASCII(COMET_ASCII_WARNING_COL), args...);
  }

  template <typename... Targs>
  void Info(LoggerType type, const Targs&... args) {
    Log(COMET_ASCII_CATEGORY(COMET_ASCII_INFO_COL), "[",
        GetLoggerTypeLabel(type), "] [INFO] ",
        COMET_ASCII(COMET_ASCII_INFO_COL), args...);
  }

  template <typename... Targs>
  void Debug(LoggerType type, const Targs&... args) {
    Log(COMET_ASCII_CATEGORY(COMET_ASCII_DEBUG_COL), "[",
        GetLoggerTypeLabel(type), "] [DEBUG] ",
        COMET_ASCII(COMET_ASCII_DEBUG_COL), args...);
  }

 private:
  void Flush();
#ifdef COMET_LOG_IS_FIBER_PREFIX
  void PopulateFiberPrefix(schar* buffer, usize buffer_len);
#endif  // COMET_LOG_IS_FIBER_PREFIX

  struct Buffer {
    static constexpr auto kBufferSize{4096};
    schar data[kBufferSize];
    std::atomic<usize> write_index{0};
    std::atomic<usize> active_writer_count{0};
    std::atomic<bool> is_flush_requested{false};
  };

  static constexpr auto kBufferCount_{2};
  static constexpr auto kFlushIntervalInMs_{100};
  StaticArray<Buffer, kBufferCount_> buffers_{};
  std::atomic<usize> current_buffer_index_{0};
  std::atomic<bool> is_initialized_{false};
  std::atomic<bool> is_running_{false};
  thread::Thread flush_thread_{};

  Logger();

  void ListenToFlushRequests();
};

#ifndef COMET_DEBUG
#define COMET_LOG_INFO(logger_type, ...)
#define COMET_LOG_DEBUG(logger_type, ...)
#define COMET_LOG_WARNING(logger_type, ...)

#define COMET_LOG_GLOBAL_INFO(...)
#define COMET_LOG_GLOBAL_DEBUG(...)
#define COMET_LOG_GLOBAL_WARNING(...)

#define COMET_LOG_ANIMATION_INFO(...)
#define COMET_LOG_ANIMATION_DEBUG(...)
#define COMET_LOG_ANIMATION_WARNING(...)

#define COMET_LOG_CORE_INFO(...)
#define COMET_LOG_CORE_DEBUG(...)
#define COMET_LOG_CORE_WARNING(...)

#define COMET_LOG_EVENT_INFO(...)
#define COMET_LOG_EVENT_DEBUG(...)
#define COMET_LOG_EVENT_WARNING(...)

#define COMET_LOG_ENTITY_INFO(...)
#define COMET_LOG_ENTITY_DEBUG(...)
#define COMET_LOG_ENTITY_WARNING(...)

#define COMET_LOG_GEOMETRY_INFO(...)
#define COMET_LOG_GEOMETRY_DEBUG(...)
#define COMET_LOG_GEOMETRY_WARNING(...)

#define COMET_LOG_INPUT_INFO(...)
#define COMET_LOG_INPUT_DEBUG(...)
#define COMET_LOG_INPUT_WARNING(...)

#define COMET_LOG_MATH_INFO(...)
#define COMET_LOG_MATH_DEBUG(...)
#define COMET_LOG_MATH_WARNING(...)

#define COMET_LOG_PHYSICS_INFO(...)
#define COMET_LOG_PHYSICS_DEBUG(...)
#define COMET_LOG_PHYSICS_WARNING(...)

#define COMET_LOG_PROFILER_INFO(...)
#define COMET_LOG_PROFILER_DEBUG(...)
#define COMET_LOG_PROFILER_WARNING(...)

#define COMET_LOG_RENDERING_INFO(...)
#define COMET_LOG_RENDERING_DEBUG(...)
#define COMET_LOG_RENDERING_WARNING(...)

#define COMET_LOG_RESOURCE_INFO(...)
#define COMET_LOG_RESOURCE_DEBUG(...)
#define COMET_LOG_RESOURCE_WARNING(...)

#define COMET_LOG_TIME_INFO(...)
#define COMET_LOG_TIME_DEBUG(...)
#define COMET_LOG_TIME_WARNING(...)
#else
#define COMET_LOG_INFO(logger_type, ...) \
  comet::Logger::Get().Info(logger_type, __VA_ARGS__)
#define COMET_LOG_DEBUG(logger_type, ...) \
  comet::Logger::Get().Debug(logger_type, __VA_ARGS__)
#define COMET_LOG_WARNING(logger_type, ...) \
  comet::Logger::Get().Warning(logger_type, __VA_ARGS__)

#define COMET_LOG_GLOBAL_INFO(...) \
  comet::Logger::Get().Info(comet::LoggerType::Global, __VA_ARGS__)
#define COMET_LOG_GLOBAL_DEBUG(...) \
  comet::Logger::Get().Debug(comet::LoggerType::Global, __VA_ARGS__)
#define COMET_LOG_GLOBAL_WARNING(...) \
  comet::Logger::Get().Warning(comet::LoggerType::Global, __VA_ARGS__)

#define COMET_LOG_ANIMATION_INFO(...) \
  comet::Logger::Get().Info(comet::LoggerType::Animation, __VA_ARGS__)
#define COMET_LOG_ANIMATION_DEBUG(...) \
  comet::Logger::Get().Debug(comet::LoggerType::Animation, __VA_ARGS__)
#define COMET_LOG_ANIMATION_WARNING(...) \
  comet::Logger::Get().Warning(comet::LoggerType::Animation, __VA_ARGS__)

#define COMET_LOG_CORE_INFO(...) \
  comet::Logger::Get().Info(comet::LoggerType::Core, __VA_ARGS__)
#define COMET_LOG_CORE_DEBUG(...) \
  comet::Logger::Get().Debug(comet::LoggerType::Core, __VA_ARGS__)
#define COMET_LOG_CORE_WARNING(...) \
  comet::Logger::Get().Warning(comet::LoggerType::Core, __VA_ARGS__)

#define COMET_LOG_EVENT_INFO(...) \
  comet::Logger::Get().Info(comet::LoggerType::Event, __VA_ARGS__)
#define COMET_LOG_EVENT_DEBUG(...) \
  comet::Logger::Get().Debug(comet::LoggerType::Event, __VA_ARGS__)
#define COMET_LOG_EVENT_WARNING(...) \
  comet::Logger::Get().Warning(comet::LoggerType::Event, __VA_ARGS__)

#define COMET_LOG_ENTITY_INFO(...) \
  comet::Logger::Get().Info(comet::LoggerType::Entity, __VA_ARGS__)
#define COMET_LOG_ENTITY_DEBUG(...) \
  comet::Logger::Get().Debug(comet::LoggerType::Entity, __VA_ARGS__)
#define COMET_LOG_ENTITY_WARNING(...) \
  comet::Logger::Get().Warning(comet::LoggerType::Entity, __VA_ARGS__)

#define COMET_LOG_GEOMETRY_INFO(...) \
  comet::Logger::Get().Info(comet::LoggerType::Geometry, __VA_ARGS__)
#define COMET_LOG_GEOMETRY_DEBUG(...) \
  comet::Logger::Get().Debug(comet::LoggerType::Geometry, __VA_ARGS__)
#define COMET_LOG_GEOMETRY_WARNING(...) \
  comet::Logger::Get().Warning(comet::LoggerType::Geometry, __VA_ARGS__)

#define COMET_LOG_INPUT_INFO(...) \
  comet::Logger::Get().Info(comet::LoggerType::Input, __VA_ARGS__)
#define COMET_LOG_INPUT_DEBUG(...) \
  comet::Logger::Get().Debug(comet::LoggerType::Input, __VA_ARGS__)
#define COMET_LOG_INPUT_WARNING(...) \
  comet::Logger::Get().Warning(comet::LoggerType::Input, __VA_ARGS__)

#define COMET_LOG_MATH_INFO(...) \
  comet::Logger::Get().Info(comet::LoggerType::Math, __VA_ARGS__)
#define COMET_LOG_MATH_DEBUG(...) \
  comet::Logger::Get().Debug(comet::LoggerType::Math, __VA_ARGS__)
#define COMET_LOG_MATH_WARNING(...) \
  comet::Logger::Get().Warning(comet::LoggerType::Math, __VA_ARGS__)

#define COMET_LOG_PHYSICS_INFO(...) \
  comet::Logger::Get().Info(comet::LoggerType::Physics, __VA_ARGS__)
#define COMET_LOG_PHYSICS_DEBUG(...) \
  comet::Logger::Get().Debug(comet::LoggerType::Physics, __VA_ARGS__)
#define COMET_LOG_PHYSICS_WARNING(...) \
  comet::Logger::Get().Warning(comet::LoggerType::Physics, __VA_ARGS__)

#define COMET_LOG_PROFILER_INFO(...) \
  comet::Logger::Get().Info(comet::LoggerType::Profiler, __VA_ARGS__)
#define COMET_LOG_PROFILER_DEBUG(...) \
  comet::Logger::Get().Debug(comet::LoggerType::Profiler, __VA_ARGS__)
#define COMET_LOG_PROFILER_WARNING(...) \
  comet::Logger::Get().Warning(comet::LoggerType::Profiler, __VA_ARGS__)

#define COMET_LOG_RENDERING_INFO(...) \
  comet::Logger::Get().Info(comet::LoggerType::Rendering, __VA_ARGS__)
#define COMET_LOG_RENDERING_DEBUG(...) \
  comet::Logger::Get().Debug(comet::LoggerType::Rendering, __VA_ARGS__)
#define COMET_LOG_RENDERING_WARNING(...) \
  comet::Logger::Get().Warning(comet::LoggerType::Rendering, __VA_ARGS__)

#define COMET_LOG_RESOURCE_INFO(...) \
  comet::Logger::Get().Info(comet::LoggerType::Resource, __VA_ARGS__)
#define COMET_LOG_RESOURCE_DEBUG(...) \
  comet::Logger::Get().Debug(comet::LoggerType::Resource, __VA_ARGS__)
#define COMET_LOG_RESOURCE_WARNING(...) \
  comet::Logger::Get().Warning(comet::LoggerType::Resource, __VA_ARGS__)

#define COMET_LOG_TIME_INFO(...) \
  comet::Logger::Get().Info(comet::LoggerType::Time, __VA_ARGS__)
#define COMET_LOG_TIME_DEBUG(...) \
  comet::Logger::Get().Debug(comet::LoggerType::Time, __VA_ARGS__)
#define COMET_LOG_TIME_WARNING(...) \
  comet::Logger::Get().Warning(comet::LoggerType::Time, __VA_ARGS__)
#endif  // !COMET_DEBUG

#define COMET_LOG_INITIALIZE() comet::Logger::Get()
#define COMET_LOG_DESTROY() comet::Logger::Get().Destroy();

#define COMET_LOG_ERROR(logger_type, ...) \
  comet::Logger::Get().Error(logger_type, __VA_ARGS__)
#define COMET_LOG_GLOBAL_ERROR(...) \
  comet::Logger::Get().Error(comet::LoggerType::Global, __VA_ARGS__)
#define COMET_LOG_ANIMATION_ERROR(...) \
  comet::Logger::Get().Error(comet::LoggerType::Animation, __VA_ARGS__)
#define COMET_LOG_CORE_ERROR(...) \
  comet::Logger::Get().Error(comet::LoggerType::Core, __VA_ARGS__)
#define COMET_LOG_EVENT_ERROR(...) \
  comet::Logger::Get().Error(comet::LoggerType::Event, __VA_ARGS__)
#define COMET_LOG_ENTITY_ERROR(...) \
  comet::Logger::Get().Error(comet::LoggerType::Entity, __VA_ARGS__)
#define COMET_LOG_GEOMETRY_ERROR(...) \
  comet::Logger::Get().Error(comet::LoggerType::Geometry, __VA_ARGS__)
#define COMET_LOG_INPUT_ERROR(...) \
  comet::Logger::Get().Error(comet::LoggerType::Input, __VA_ARGS__)
#define COMET_LOG_MATH_ERROR(...) \
  comet::Logger::Get().Error(comet::LoggerType::Math, __VA_ARGS__)
#define COMET_LOG_PHYSICS_ERROR(...) \
  comet::Logger::Get().Error(comet::LoggerType::Physics, __VA_ARGS__)
#define COMET_LOG_PROFILER_ERROR(...) \
  comet::Logger::Get().Error(comet::LoggerType::Profiler, __VA_ARGS__)
#define COMET_LOG_RENDERING_ERROR(...) \
  comet::Logger::Get().Error(comet::LoggerType::Rendering, __VA_ARGS__)
#define COMET_LOG_RESOURCE_ERROR(...) \
  comet::Logger::Get().Error(comet::LoggerType::Resource, __VA_ARGS__)
#define COMET_LOG_TIME_ERROR(...) \
  comet::Logger::Get().Error(comet::LoggerType::Time, __VA_ARGS__)
}  // namespace comet

#endif  // COMET_COMET_CORE_LOGGER_H_
