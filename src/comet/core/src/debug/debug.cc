// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/debug/debug.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <iostream>
#include <mutex>
#include <shared_mutex>

#ifdef COMET_MSVC
#include <cstdio>
#else
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <link.h>

#include <chrono>
#include <cstdlib>
#include <thread>
#endif  // COMET_MSVC
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/string/c_string.h"
#include "comet/core/compiler.h"

#ifdef COMET_MSVC
#include "comet/core/windows.h"
#endif  // COMET_MSVC

#include "comet/core/memory/memory_utils.h"

#ifdef COMET_CHECK_STACK_OVERFLOWS
#ifdef COMET_UNIX

#include "comet/core/fiber/fiber_context.h"
#endif  // COMET_UNIX
#endif  // COMET_CHECK_STACK_OVERFLOWS

namespace comet {
namespace debug {
namespace internal {
// A shared lock pins the provider while it is running. Unregistering waits
// for active providers to finish before runtime managers are destroyed.
std::shared_mutex diagnostics_mutex;
CriticalErrorDiagnosticsCallback diagnostics_callback{nullptr};
thread_local bool is_reporting_critical_error{false};
}  // namespace internal

void SetCriticalErrorDiagnostics(CriticalErrorDiagnosticsCallback callback) {
  std::unique_lock<std::shared_mutex> lock{internal::diagnostics_mutex};
  internal::diagnostics_callback = callback;
}

void UnsetCriticalErrorDiagnostics() {
  SetCriticalErrorDiagnostics(nullptr);
}

// TODO(m4jr0): Handle critical error properly.
void HandleCriticalError() {
  std::cerr << "debug::HandleCriticalError: critical failure\n";

  // A nested assertion must not recursively invoke the provider.
  if (!internal::is_reporting_critical_error) {
    internal::is_reporting_critical_error = true;
    // try_lock prevents deadlock if an assertion occurs during registration.
    std::shared_lock<std::shared_mutex> lock{internal::diagnostics_mutex,
                                            std::try_to_lock};
    if (lock.owns_lock() && internal::diagnostics_callback != nullptr) {
      internal::diagnostics_callback(std::cerr);
    }
    internal::is_reporting_critical_error = false;
  }

  constexpr auto kBufferLen{4096};
  schar buffer[kBufferLen]{'\0'};
  GenerateStackTrace(buffer, kBufferLen);

  std::cerr << buffer << '\n';
}

void GenerateStackTrace(schar* buffer, usize buffer_len) {
  constexpr auto* kPrefix{"stacktrace:\n"};
  constexpr auto kPrefixLen{GetLength(kPrefix)};
  COMET_CASSERT(buffer_len > kPrefixLen, "buffer provided is too small");

  Copy(buffer, kPrefix, kPrefixLen);
  buffer += kPrefixLen;
  buffer_len -= kPrefixLen;

#ifdef COMET_MSVC
#ifndef COMET_DEBUG
  constexpr auto* kReleaseStr{"not available in release builds"};
  constexpr auto kkReleaseStrLen{GetLength(kReleaseStr)};
  COMET_CASSERT(buffer_len > kkReleaseStrLen, "buffer provided is too small");
  Copy(buffer, kReleaseStr, kkReleaseStrLen);
  return;
#else
  constexpr auto max_frame_count{128};
  void* frames[max_frame_count]{nullptr};
  const auto frame_count{
      CaptureStackBackTrace(0, max_frame_count, frames, nullptr)};
  const auto process_handle{GetCurrentProcess()};

  try {
    if (!SymInitialize(process_handle, nullptr, true)) {
      std::cerr << "debug::GenerateStackTrace: could not generate stacktrace. "
                   "error code: "
                << GetLastError() << '\n';
      return;
    }
  } catch (...) {
    std::cerr << "debug::GenerateStackTrace: could not generate stacktrace. "
                 "error code: "
              << GetLastError() << '\n';
    return;
  }

  constexpr usize kHexAddressLen{memory::kHexAddressLength};
  constexpr usize kHexAddressBufferLen{memory::kHexAddressBufferLen};
  schar hex_address[kHexAddressBufferLen]{'\0'};
  constexpr usize kDepthBufferLen{4};
  schar depth[kDepthBufferLen]{'\0'};
  usize depth_len{0};

  // Start from index 1 to avoid printing current function.
  for (u16 i{1}; i < frame_count; ++i) {
    const auto address{reinterpret_cast<u64>(frames[i])};
    schar symbol_buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)]{
        '\0'};

    auto* symbol{reinterpret_cast<SYMBOL_INFO*>(symbol_buffer)};
    symbol->MaxNameLen = MAX_SYM_NAME;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    if (!SymFromAddr(process_handle, address, nullptr, symbol)) {
      break;
    }

    memory::ConvertAddressToHex(symbol->Address, hex_address,
                                kHexAddressBufferLen);
    ConvertToStr(i - 1, depth, kDepthBufferLen, &depth_len);

    const auto required_len{7 + symbol->NameLen + kHexAddressLen + depth_len};

    if (buffer_len <= required_len) {
      break;
    }

    buffer[0] = '\t';
    Copy(buffer, depth, depth_len, 1);
    Copy(buffer, ": [", 3, 1 + depth_len);
    Copy(buffer, hex_address, kHexAddressLen, 4 + depth_len);
    Copy(buffer, "] ", 2, kHexAddressLen + 4 + depth_len);
    Copy(buffer, symbol->Name, symbol->NameLen, kHexAddressLen + 6 + depth_len);
    buffer[required_len - 1] = '\n';
    buffer_len -= required_len;
    buffer += required_len;
  }

  try {
    if (!SymCleanup(process_handle)) {
      std::cerr << "debug::GenerateStackTrace: could not generate stacktrace. "
                   "error code: "
                << GetLastError() << '\n';
      return;
    }
  } catch (...) {
    std::cerr << "debug::GenerateStackTrace: could not generate stacktrace. "
                 "error code: "
              << GetLastError() << '\n';
    return;
  }

  COMET_CASSERT(buffer_len > 1, "buffer provided is too small");
  buffer[0] = '\0';
#endif  // !COMET_DEBUG
#else
  constexpr auto max_frame_count{128};
  void* frames[max_frame_count]{nullptr};
  const auto frame_count{
      static_cast<usize>(backtrace(frames, max_frame_count))};
  char** symbols{backtrace_symbols(frames, frame_count)};
  if (symbols == nullptr) {
    buffer[0] = '\0';
    return;
  }

  constexpr usize kDepthBufferLen{4};
  schar depth[kDepthBufferLen]{'\0'};
  usize depth_len{0};

  constexpr usize kHexAddressLen{memory::kHexAddressLength};
  constexpr usize kHexAddressBufferLen{kHexAddressLen + 1};
  schar hex_address[kHexAddressBufferLen]{'\0'};
  schar module_hex_address[kHexAddressBufferLen]{'\0'};

  // Start from index 2 to avoid printing current function.
  for (usize i{2}; i < frame_count; ++i) {
    Dl_info info;

    if (dladdr(frames[i], &info) && info.dli_sname) {
      int status;
      auto* demangled{
          abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status)};
      const auto* function_name{(status == 0) ? demangled : info.dli_sname};
      const auto function_name_len{GetLength(function_name)};
      const auto function_address{reinterpret_cast<uptr>(info.dli_saddr)};
      const auto module_address{reinterpret_cast<uptr>(info.dli_fbase)};

      memory::ConvertAddressToHex(function_address, hex_address,
                                  kHexAddressBufferLen);
      memory::ConvertAddressToHex(module_address, module_hex_address,
                                  kHexAddressBufferLen);
      ConvertToStr(i - 1, depth, kDepthBufferLen, &depth_len);

      const auto required_len{9 + function_name_len + kHexAddressLen * 2 +
                              depth_len};

      if (buffer_len <= required_len) {
        break;
      }

      buffer[0] = '\t';
      Copy(buffer, depth, depth_len, 1);
      Copy(buffer, ": [", 3, 1 + depth_len);
      Copy(buffer, hex_address, kHexAddressLen, 4 + depth_len);
      Copy(buffer, ", ", 2, kHexAddressLen + 4 + depth_len);
      Copy(buffer, module_hex_address, kHexAddressLen,
           kHexAddressLen + 6 + depth_len);
      Copy(buffer, "] ", 2, kHexAddressLen * 2 + 6 + depth_len);
      Copy(buffer, function_name, function_name_len,
           kHexAddressLen * 2 + 8 + depth_len);
      buffer[required_len - 1] = '\n';
      buffer_len -= required_len;
      buffer += required_len;

      free(demangled);
    } else {
      const auto symbol_len{GetLength(symbols[i])};
      ConvertToStr(i - 1, depth, kDepthBufferLen, &depth_len);
      const auto required_len{4 + symbol_len + depth_len};

      if (buffer_len <= required_len) {
        break;
      }

      buffer[0] = '\t';
      Copy(buffer, depth, depth_len, 1);
      Copy(buffer, ": ", 2, 1 + depth_len);
      Copy(buffer, symbols[i], symbol_len, 3 + depth_len);
      buffer[required_len - 1] = '\n';
      buffer_len -= required_len;
      buffer += required_len;
    }
  }

  free(symbols);
  COMET_CASSERT(buffer_len > 1, "buffer provided is too small");
  buffer[0] = '\0';
#endif  // COMET_MSVC
}
}  // namespace debug
}  // namespace comet
