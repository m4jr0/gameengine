// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be it in the LICENSE file.

#include "logger.h"

#include "comet/core/job/scheduler_utils.h"

namespace comet {
void Logger::Flush() {
  if (buffer_index_ == 0) {
    return;
  }

  buffer_[buffer_index_] = '\0';
  std::cout << buffer_;
  buffer_index_ = 0;
}

Logger& Logger::Get() {
  static Logger singleton{};
  return singleton;
}

Logger::Logger() { ScheduleLogsProcessing(); }

void Logger::ProcessLogs() {
  job::FiberAwareLockGuard lock{buffer_lock_};
  Flush();
}

void Logger::ScheduleLogsProcessing() {
  auto& scheduler{job::Scheduler::Get()};
  last_counter_ = scheduler.AllocateCounter();
  scheduler.Kick(job::GenerateIOJobDescr(
      OnLogProcessingRequest, reinterpret_cast<job::ParamsHandle>(this),
      last_counter_));
}

void Logger::OnLogProcessingRequest(job::ParamsHandle params_handle) {
  auto* logger{reinterpret_cast<Logger*>(params_handle)};
  COMET_ASSERT(logger->last_counter_ != nullptr, "Last counter is null!");
  job::Scheduler::Get().FreeCounter(logger->last_counter_);
  logger->last_counter_ = nullptr;
  logger->ProcessLogs();
  logger->ScheduleLogsProcessing();
}

const schar* GetLoggerTypeLabel(LoggerType type) {
  switch (type) {
    case LoggerType::Global:
      return "Global";
    case LoggerType::Animation:
      return "Animation";
    case LoggerType::Core:
      return "Core";
    case LoggerType::Event:
      return "Event";
    case LoggerType::Entity:
      return "Entity";
    case LoggerType::Input:
      return "Input";
    case LoggerType::Math:
      return "Math";
    case LoggerType::Physics:
      return "Physics";
    case LoggerType::Profiler:
      return "Profiler";
    case LoggerType::Rendering:
      return "Rendering";
    case LoggerType::Resource:
      return "Resource";
    case LoggerType::Time:
      return "Time";
    default:
      return "???";
  }
}

void Logger::AddToBuffer(CTStringView arg) {
  // TODO(m4jr0): Use buffer from allocator.
  constexpr auto kSize{512};
  schar tmp[kSize]{'\0'};
  auto len{kSize < arg.GetLengthWithNullTerminator()
               ? kSize
               : arg.GetLengthWithNullTerminator()};
  Copy(tmp, arg.GetCTStr(), len - 1);
  tmp[len] = '\0';
  AddToBuffer(tmp);
}

void Logger::AddToBuffer(std::string_view arg) {
  job::FiberAwareLockGuard lock{buffer_lock_};
  auto len{arg.size()};

  // Take both \0 and \n into account.
  if (kBufferSize_ - buffer_index_ <= len + 1) {
    Flush();
  }

  std::memcpy(buffer_ + buffer_index_, arg.data(), len);
  buffer_index_ += len;
}
}  // namespace comet
