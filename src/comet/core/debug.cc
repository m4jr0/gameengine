// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "debug.h"

#include <iostream>

#include "comet/core/compiler.h"

#ifdef COMET_MSVC
#include <cstdio>

#include "comet/core/windows.h"
#else
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <link.h>

#include <cstdlib>
#endif  // COMET_MSVC

#include "comet/core/logger.h"
#include "comet/core/memory/memory.h"

#ifdef COMET_CHECK_STACK_OVERFLOWS
#ifdef COMET_UNIX

#include "comet/core/concurrency/fiber/fiber_context.h"
#endif  // COMET_UNIX
#endif  // COMET_CHECK_STACK_OVERFLOWS

namespace comet {
namespace debug {
// TODO(m4jr0): Handle critical error properly.
void HandleCriticalError() {
  std::cerr
      << "A critical error has occurred. The application must now terminate.\n";

  constexpr auto kBufferLen{4096};
  schar buffer[kBufferLen]{'\0'};
  GenerateStackTrace(buffer, kBufferLen);

  std::cerr << buffer << '\n';
  std::cerr << "Aborting...\n";
}

void GenerateStackTrace(schar* buffer, usize buffer_len) {
  constexpr auto* kPrefix{"Stacktrace:\n"};
  constexpr auto kPrefixLen{GetLength(kPrefix)};

  COMET_CASSERT(buffer_len > kPrefixLen, "Buffer provided is too small!");
  Copy(buffer, kPrefix, kPrefixLen);
  buffer += kPrefixLen;
  buffer_len -= kPrefixLen;

#ifdef COMET_MSVC
#ifndef COMET_DEBUG
  constexpr auto* kReleaseStr{"Not available in release builds."};
  constexpr auto kkReleaseStrLen{GetLength(kReleaseStr)};
  COMET_CASSERT(buffer_len > kReleaseStr, "Buffer provided is too small!");
  Copy(buffer, kReleaseStr, kkReleaseStrLen);
  return;
#else
  constexpr auto max_frame_count{128};
  void* frames[max_frame_count]{nullptr};
  auto frame_count{CaptureStackBackTrace(0, max_frame_count, frames, nullptr)};
  auto process_handle{GetCurrentProcess()};
  SymInitialize(process_handle, nullptr, true);
  constexpr usize kHexAddressLen{memory::kHexAddressLength};
  constexpr usize kHexAddressBufferLen{kHexAddressLen + 1};
  schar hex_address[kHexAddressBufferLen]{'\0'};
  constexpr usize kDepthBufferLen{4};
  schar depth[kDepthBufferLen]{'\0'};
  usize depth_len{0};

  // Start from index 1 to avoid printing current function.
  for (u16 i{1}; i < frame_count; ++i) {
    auto address{reinterpret_cast<u64>(frames[i])};
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

    auto required_len{7 + symbol->NameLen + kHexAddressLen + depth_len};

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

  SymCleanup(process_handle);
  COMET_CASSERT(buffer_len > 1, "Buffer provided is too small!");
  buffer[0] = '\0';
#endif  // !COMET_DEBUG
#else
  constexpr auto max_frame_count{128};
  void* frames[max_frame_count]{nullptr};
  auto frame_count{static_cast<usize>(backtrace(frames, max_frame_count))};
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
      auto function_name_len{GetLength(function_name)};
      auto function_address{reinterpret_cast<uptr>(info.dli_saddr)};
      auto module_address{reinterpret_cast<uptr>(info.dli_fbase)};

      memory::ConvertAddressToHex(function_address, hex_address,
                                  kHexAddressBufferLen);
      memory::ConvertAddressToHex(module_address, module_hex_address,
                                  kHexAddressBufferLen);
      ConvertToStr(i - 1, depth, kDepthBufferLen, &depth_len);

      auto required_len{9 + function_name_len + kHexAddressLen * 2 + depth_len};

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
      auto symbol_len{GetLength(symbols[i])};
      ConvertToStr(i - 1, depth, kDepthBufferLen, &depth_len);
      auto required_len{4 + symbol_len + depth_len};

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
  COMET_CASSERT(buffer_len > 1, "Buffer provided is too small!");
  buffer[0] = '\0';
#endif  // COMET_MSVC
}
}  // namespace debug
}  // namespace comet
