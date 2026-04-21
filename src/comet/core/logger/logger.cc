// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be it in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "logger.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/concurrency/fiber/fiber_context.h"
#include "comet/core/concurrency/thread/thread_context.h"
#include "comet/core/memory/memory_utils.h"

namespace comet {
const schar* GetLogLevelColor(LogLevel level) {
  switch (level) {
    case LogLevel::Info:
      return COMET_ASCII(COMET_ASCII_INFO_COL);
    case LogLevel::Warning:
      return COMET_ASCII(COMET_ASCII_WARNING_COL);
    case LogLevel::Error:
      return COMET_ASCII(COMET_ASCII_ERROR_COL);
    case LogLevel::Debug:
      return COMET_ASCII(COMET_ASCII_DEBUG_COL);
    default:
      return COMET_ASCII(COMET_ASCII_NORMAL_COL);
  }
}

const schar* GetLogLevelCategoryColor(LogLevel level) {
  switch (level) {
    case LogLevel::Info:
      return COMET_ASCII_CATEGORY(COMET_ASCII_INFO_COL);
    case LogLevel::Warning:
      return COMET_ASCII_CATEGORY(COMET_ASCII_WARNING_COL);
    case LogLevel::Error:
      return COMET_ASCII_CATEGORY(COMET_ASCII_ERROR_COL);
    case LogLevel::Debug:
      return COMET_ASCII_CATEGORY(COMET_ASCII_DEBUG_COL);
    default:
      return COMET_ASCII_CATEGORY(COMET_ASCII_NORMAL_COL);
  }
}

void Logger::Initialize() {
  COMET_ASSERT(!is_initialized_.load(std::memory_order_acquire),
               "Logger::Initialize", "logger is already initialized");
  is_running_.store(true, std::memory_order_release);
  is_initialized_.store(true, std::memory_order_release);
  flush_thread_.Run(&Logger::ListenToFlushRequests, this);
}

void Logger::Destroy() {
  COMET_ASSERT(is_initialized_.load(std::memory_order_acquire),
               "Logger::Destroy", "logger is not initialized");
  ShutdownInternal();
}

Logger& Logger::Get() {
  static Logger singleton{};
  return singleton;
}

Logger::~Logger() {
  COMET_ASSERT(!is_initialized_.load(std::memory_order_acquire),
               "Logger::~Logger", "logger is still initialized");
  ShutdownInternal();
}

void Logger::AddToBuffer(schar* buffer, usize len, usize& offset,
                         const TString& arg) {
#ifdef COMET_WIDE_TCHAR
  AddToBuffer(buffer, len, offset, arg.GetCTStr());
#else
  AddToBuffer(buffer, len, offset, std::string_view{arg.GetCTStr()});
#endif  // COMET_WIDE_TCHAR
}

void Logger::AddToBuffer(schar* buffer, usize len, usize& offset,
                         const schar* arg) {
  AddToBuffer(buffer, len, offset, std::string_view{arg});
}

void Logger::AddToBuffer(schar* buffer, usize len, usize& offset,
                         const wchar* arg) {
  // TODO(m4jr0): Use buffer from allocator.
  constexpr auto kSize{512};
  schar tmp[kSize]{'\0'};
  const auto arg_len{GetLength(arg) + 1};
  const auto tmp_len{kSize < arg_len ? kSize : arg_len};
  Copy(tmp, arg, tmp_len - 1);
  tmp[tmp_len - 1] = '\0';
  AddToBuffer(buffer, len, offset, std::string_view{tmp});
}

void Logger::AddToBuffer(schar* buffer, usize len, usize& offset,
                         CTStringView arg) {
  AddToBuffer(buffer, len, offset, arg.GetCTStr());
}

void Logger::AddToBuffer(schar* buffer, usize len, usize& offset,
                         std::string_view arg) {
  auto len_to_copy{arg.size()};

  // Take both \0 and \n into account.
  if (offset + len_to_copy >= len - 1) {
    len_to_copy = len - offset - 2;
  }

  if (len_to_copy == 0) {
    return;
  }

  Copy(buffer, arg.data(), len_to_copy, offset);
  offset += len_to_copy;
}

void Logger::Send(const schar* str, usize len) {
  for (;;) {
    const auto current_buffer_index{
        current_buffer_index_.load(std::memory_order_acquire)};

    auto& buffer{buffers_[current_buffer_index]};
    buffer.active_writer_count.fetch_add(1, std::memory_order_release);

    if (current_buffer_index !=
        current_buffer_index_.load(std::memory_order_acquire)) {
      buffer.active_writer_count.fetch_sub(1, std::memory_order_release);
      continue;
    }

    const auto old_current_len{
        buffer.write_index.fetch_add(len, std::memory_order_acq_rel)};

    // Take \0 into account.
    if (old_current_len + len + 1 > Buffer::kBufferSize) {
      buffer.is_flush_requested.store(true, std::memory_order_release);
      buffer.active_writer_count.fetch_sub(1, std::memory_order_release);
      continue;
    }

    memory::CopyMemory(buffer.data + old_current_len, str, len);
    buffer.active_writer_count.fetch_sub(1, std::memory_order_release);
    break;
  }
}

void Logger::ShutdownInternal() {
  const auto was_initialized{
      is_initialized_.exchange(false, std::memory_order_acq_rel)};

  if (!was_initialized) {
    return;
  }

  is_running_.store(false, std::memory_order_release);
  flush_thread_.TryJoin();

  // Drain both buffers after the flush thread is gone.
  Flush();
  Flush();
}

void Logger::ListenToFlushRequests() {
  while (!is_initialized_.load(std::memory_order_acquire)) {
    thread::Yield();
  }

  flush_chrono_.Start(kFlushIntervalInMs_);

  while (is_running_.load(std::memory_order_acquire)) {
    thread::Yield();

    // current_buffer_index_ will always be synchronized by the flush thread
    // (only this thread modifies this value).
    const auto current_buffer_index{
        current_buffer_index_.load(std::memory_order_relaxed)};

    if (flush_chrono_.IsFinished() ||
        buffers_[current_buffer_index].is_flush_requested.load(
            std::memory_order_acquire)) {
      Flush();
    }
  }
}

void Logger::Flush() {
  flush_chrono_.Restart();

  // current_buffer_index_ will always be synchronized by the flush thread (only
  // this thread modifies this value).
  const auto current_buffer_index{
      current_buffer_index_.load(std::memory_order_relaxed)};
  current_buffer_index_.exchange((current_buffer_index + 1) % kBufferCount_,
                                 std::memory_order_release);

  auto& buffer{buffers_[current_buffer_index]};

  while (buffer.active_writer_count.load(std::memory_order_acquire) != 0) {
    thread::Yield();
  }

  const auto len{buffers_[current_buffer_index].write_index.load(
      std::memory_order_acquire)};

  if (len == 0) {
    return;
  }

  auto* str{buffer.data};
  std::cout << str;
  Clear(str, Buffer::kBufferSize);
  buffer.write_index.store(0, std::memory_order_release);
  buffer.is_flush_requested.store(false, std::memory_order_release);
}

#ifdef COMET_LOG_USE_FIBER_PREFIX
void Logger::PopulateFiberPrefix(schar* buffer, usize buffer_len) {
  // This code doesn't check for buffer overflows.
  // That's fine for now, as it's only used for debugging.
  const auto thread_id{thread::GetThreadId()};

  buffer[0] = '[';
  ++buffer;
  --buffer_len;
  usize cur_len;
  ConvertToStr(thread_id, buffer, buffer_len, &cur_len);
  buffer += cur_len;
  Copy(buffer, "] [", 3);
  buffer += 3;
  cur_len = 0;

  if (fiber::IsFiber()) {
    const auto* fiber{fiber::GetFiber()};
    const auto fiber_id{fiber->GetId()};
    ConvertToStr(fiber_id, buffer, buffer_len, &cur_len);
    buffer += cur_len;
#ifdef COMET_FIBER_DEBUG_LABEL
    const auto* fiber_label{fiber->GetDebugLabel()};
    const auto fiber_label_len{GetLength(fiber_label)};
    buffer[0] = '#';
    ++buffer;
    Copy(buffer, fiber_label, fiber_label_len);
    buffer += fiber_label_len;
#endif  // COMET_FIBER_DEBUG_LABEL
  } else {
    Copy(buffer, "I/O", 3);
    buffer += 3;
  }

  Copy(buffer, "] | \0", 5);
}
#endif  // COMET_LOG_USE_FIBER_PREFIX
}  // namespace comet
