// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_LOGGER_LOGGER_H_
#define COMET_COMET_CORE_LOGGER_LOGGER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
#include <concepts>
#include <string_view>
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/thread/thread.h"
#include "comet/core/essentials.h"
#include "comet/core/logger/logger_label.h"
#include "comet/core/logger/logger_type.h"
#include "comet/core/type/array.h"
#include "comet/core/type/buffer_formatter.h"
#include "comet/core/type/tstring.h"
#include "comet/time/chrono.h"

// If issues arise with current terminal, comment this line.
#define COMET_TERMINAL_COLORS

#define COMET_ASCII_PREFIX "\033["
#define COMET_ASCII_SUFFIX "m"
#define COMET_ASCII_ATTR_SUFFIX ";"
#define COMET_ASCII_EMPHASIS_ATTR "1" COMET_ASCII_ATTR_SUFFIX

#define COMET_ASCII_NORMAL_COL "0"
#define COMET_ASCII_INFO_COL COMET_ASCII_NORMAL_COL
#define COMET_ASCII_ERROR_COL "31"
#define COMET_ASCII_WARNING_COL "33"
#define COMET_ASCII_DEBUG_COL "90"

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
template <typename T>
concept LogFieldKey = std::is_convertible_v<T, const schar*> ||
                      std::is_convertible_v<T, const wchar*> ||
                      std::is_convertible_v<T, CTStringView> ||
                      std::is_convertible_v<T, std::string_view>;

const schar* GetLogLevelColor(LogLevel level);
const schar* GetLogLevelCategoryColor(LogLevel level);

class Logger final {
  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> must be always lock-free");
  static_assert(std::atomic<bool>::is_always_lock_free,
                "std::atomic<bool> must be always lock-free");

 public:
  void Initialize();
  void Destroy();

 private:
  void AddToBuffer(schar* buffer, usize len, usize& offset, const TString& arg);
  void AddToBuffer(schar* buffer, usize len, usize& offset, const schar* arg);
  void AddToBuffer(schar* buffer, usize len, usize& offset, const wchar* arg);
  void AddToBuffer(schar* buffer, usize len, usize& offset, CTStringView arg);
  void AddToBuffer(schar* buffer, usize len, usize& offset,
                   std::string_view arg);

  template <typename T>
    requires HasBufferFormatter<T>
  void AddToBuffer(schar* buffer, usize buffer_len, usize& offset,
                   const T& arg) {
    constexpr usize kTmpSize{128};
    schar tmp[kTmpSize]{'\0'};
    const auto written{BufferFormatter<T>::Format(arg, tmp, kTmpSize)};
    AddToBuffer(buffer, buffer_len, offset, std::string_view{tmp, written});
  }

  void Send(const schar* buffer, usize buffer_len);

 public:
  template <typename T>
  typename std::enable_if<
      std::is_fundamental<T>::value && !is_char_pointer<T>::value, void>::type
  AddToBuffer(schar* buffer, usize buffer_len, usize& offset, const T& arg) {
    constexpr auto kSize{512};
    schar tmp[kSize]{'\0'};

    usize out_len;
    ConvertToStr(arg, tmp, kSize, &out_len);
    AddToBuffer(buffer, buffer_len, offset, tmp);
  }

 private:
  template <typename... Targs>
  void ProcessArgs(schar* buffer, usize buffer_len, usize& offset,
                   const Targs&... args) {
    (AddToBuffer(buffer, buffer_len, offset, args), ...);
  }

  void ProcessFieldsImpl(schar*, usize, usize&) {}

  template <typename TKey, typename TValue, typename... TRest>
    requires(LogFieldKey<TKey>)
  void ProcessFieldsImpl(schar* buffer, usize buffer_len, usize& offset,
                         const TKey& key, const TValue& value,
                         const TRest&... rest) {
    AddToBuffer(buffer, buffer_len, offset, key);
    AddToBuffer(buffer, buffer_len, offset, "=");
    AddToBuffer(buffer, buffer_len, offset, value);

    if constexpr (sizeof...(rest) > 0) {
      AddToBuffer(buffer, buffer_len, offset, ", ");
      ProcessFieldsImpl(buffer, buffer_len, offset, rest...);
    }
  }

  template <typename... TFields>
  void ProcessFields(schar* buffer, usize buffer_len, usize& offset,
                     const TFields&... fields) {
    static_assert(sizeof...(fields) % 2 == 0,
                  "log fields must be provided as key/value pairs");

    if constexpr (sizeof...(fields) > 0) {
      AddToBuffer(buffer, buffer_len, offset, " (");
      ProcessFieldsImpl(buffer, buffer_len, offset, fields...);
      AddToBuffer(buffer, buffer_len, offset, ")");
    }
  }

 public:
  static Logger& Get();

  Logger(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&) = delete;
  ~Logger();

  template <typename... TFields>
  void Log(LoggerType type, LogLevel level,
           [[maybe_unused]] const schar* prefix, const schar* message,
           const TFields&... fields) {
    static_assert(sizeof...(fields) % 2 == 0,
                  "log fields must be provided as key/value pairs");

    constexpr auto kSize{1024};
    schar tmp[kSize]{'\0'};
    usize offset{0};

#ifdef COMET_LOG_USE_FIBER_PREFIX
    constexpr auto kFiberPrefixSize{256};
    schar fiber_prefix[kFiberPrefixSize]{'\0'};
    PopulateFiberPrefix(fiber_prefix, kFiberPrefixSize);
    ProcessArgs(tmp, kSize, offset, fiber_prefix);
#endif  // COMET_LOG_USE_FIBER_PREFIX

    ProcessArgs(tmp, kSize, offset, GetLogLevelCategoryColor(level), "[",
                GetLoggerTypeLabel(type), "] ", "[", GetLogLevelLabel(level),
                "] ",
#ifdef COMET_LOG_USE_CONTEXT_PREFIX
                prefix, ": ",
#endif  // COMET_LOG_USE_CONTEXT_PREFIX
                GetLogLevelColor(level), message);

    ProcessFields(tmp, kSize, offset, fields...);
    ProcessArgs(tmp, kSize, offset, COMET_ASCII_RESET, "\n");

    Send(tmp, offset);
  }

  template <typename... TFields>
  void Error(LoggerType type, const schar* prefix, const schar* message,
             const TFields&... fields) {
    Log(type, LogLevel::Error, prefix, message, fields...);
  }

  template <typename... TFields>
  void Warning(LoggerType type, const schar* prefix, const schar* message,
               const TFields&... fields) {
    Log(type, LogLevel::Warning, prefix, message, fields...);
  }

  template <typename... TFields>
  void Info(LoggerType type, const schar* prefix, const schar* message,
            const TFields&... fields) {
    Log(type, LogLevel::Info, prefix, message, fields...);
  }

  template <typename... TFields>
  void Debug(LoggerType type, const schar* prefix, const schar* message,
             const TFields&... fields) {
    Log(type, LogLevel::Debug, prefix, message, fields...);
  }

 private:
  Logger() = default;

  void ShutdownInternal();

  void ListenToFlushRequests();
  void Flush();

#ifdef COMET_LOG_USE_FIBER_PREFIX
  void PopulateFiberPrefix(schar* buffer, usize buffer_len);
#endif  // COMET_LOG_USE_FIBER_PREFIX

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
  time::Chrono flush_chrono_{};
};

#ifndef COMET_DEBUG
#define COMET_LOG_DEBUG(logger_type, prefix, message, ...)
#define COMET_LOG_INFO(logger_type, prefix, message, ...)
#define COMET_LOG_WARNING(logger_type, prefix, message, ...)
#else
#define COMET_LOG_DEBUG(logger_type, prefix, message, ...) \
  comet::Logger::Get().Debug(logger_type, prefix,          \
                             message __VA_OPT__(, ) __VA_ARGS__)

#define COMET_LOG_INFO(logger_type, prefix, message, ...) \
  comet::Logger::Get().Info(logger_type, prefix,          \
                            message __VA_OPT__(, ) __VA_ARGS__)

#define COMET_LOG_WARNING(logger_type, prefix, message, ...) \
  comet::Logger::Get().Warning(logger_type, prefix,          \
                               message __VA_OPT__(, ) __VA_ARGS__)
#endif  // !COMET_DEBUG

#define COMET_LOG_INITIALIZE() comet::Logger::Get().Initialize()
#define COMET_LOG_DESTROY() comet::Logger::Get().Destroy()

#define COMET_LOG_ERROR(logger_type, prefix, message, ...) \
  comet::Logger::Get().Error(logger_type, prefix,          \
                             message __VA_OPT__(, ) __VA_ARGS__)
}  // namespace comet

#endif  // COMET_COMET_CORE_LOGGER_LOGGER_H_