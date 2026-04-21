// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_DEBUG_H_
#define COMET_COMET_CORE_DEBUG_H_

// External. ///////////////////////////////////////////////////////////////////
#include <stdio.h>

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cwchar>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/compiler.h"
#include "comet/core/define.h"
#include "comet/core/type/primitive.h"

#if defined(_MSC_VER)
#if defined(_CPPRTTI)
#define COMET_RTTI
#endif
#elif defined(__clang__)
#if __has_feature(cxx_rtti)
#define COMET_RTTI
#endif
#elif defined(__GNUG__)
#if defined(__GXX_RTTI)
#define COMET_RTTI
#endif
#endif

namespace comet {
namespace debug {
template <typename T>
concept AssertFieldKey = std::is_convertible_v<T, const schar*> ||
                         std::is_convertible_v<T, const wchar*> ||
                         std::is_convertible_v<T, std::string_view>;

void HandleCriticalError();

namespace internal {
inline usize ConvertWideCharToMb(schar* buffer,
                                 [[maybe_unused]] usize buffer_len, wchar value,
                                 std::mbstate_t& state) {
#ifdef COMET_MSVC
  usize written;
  const auto result{wcrtomb_s(&written, buffer, buffer_len, value, &state)};

  if (result != 0) {
    return kInvalidMbLen;
  }

  // wcrtomb_s may count the null terminator for ordinary chars.
  if (value != L'\0' && written > 0 && buffer[written - 1] == '\0') {
    --written;
  }

  return written;
#else
  return std::wcrtomb(buffer, value, &state);
#endif  // COMET_MSVC
}

inline void PrintValue(std::ostream& os, const schar* value) {
  os << (value != nullptr ? value : "<null>");
}

inline void PrintValue(std::ostream& os, schar* value) {
  PrintValue(os, static_cast<const schar*>(value));
}

inline void PrintValue(std::ostream& os, schar value) { os << value; }

inline void PrintValue(std::ostream& os, wchar value) {
#ifdef COMET_WIDE_TCHAR
  schar buffer[kMbCharMaxLen]{'\0'};
  std::mbstate_t state;
  const auto len{ConvertWideCharToMb(buffer, kMbCharMaxLen, value, state)};

  if (len == kInvalidMbLen) {
    os << "<?>";
    state = std::mbstate_t{};
    return;
  }

  os.write(buffer, static_cast<std::streamsize>(len));
#else
  os << static_cast<u32>(value);
#endif  // COMET_WIDE_TCHAR
}

inline void PrintValue(std::ostream& os, const wchar* value) {
  if (value == nullptr) {
    os << "<null>";
    return;
  }

  std::mbstate_t state;
  schar buffer[kMbCharMaxLen]{'\0'};

  while (*value != L'\0') {
    const auto len{ConvertWideCharToMb(buffer, kMbCharMaxLen, *value, state)};

    if (len == kInvalidMbLen) {
      os << "<?>";
      state = std::mbstate_t{};
    } else {
      os.write(buffer, static_cast<std::streamsize>(len));
    }

    ++value;
  }
}

inline void PrintValue(std::ostream& os, wchar* value) {
  PrintValue(os, static_cast<const wchar*>(value));
}

template <typename T>
inline void PrintValue(std::ostream& os, T&& value) {
  os << std::forward<T>(value);
}

inline void PrintFields(std::ostream&) {}

template <typename TKey, typename TValue, typename... TRest>
  requires AssertFieldKey<TKey>
void PrintFields(std::ostream& os, TKey&& key, TValue&& value,
                 TRest&&... rest) {
  PrintValue(os, std::forward<TKey>(key));
  os << '=';
  PrintValue(os, std::forward<TValue>(value));

  if constexpr (sizeof...(rest) > 0) {
    os << ", ";
    PrintFields(os, std::forward<TRest>(rest)...);
  }
}

template <typename... TFields>
void PrintAssertMessage(const schar* prefix, const schar* message,
                        TFields&&... fields) {
  static_assert(sizeof...(fields) % 2 == 0,
                "assert fields must be provided as key/value pairs");
  std::cerr << "[ASSERT] " << prefix << ": " << message;

  if constexpr (sizeof...(fields) > 0) {
    std::cerr << " (";
    PrintFields(std::cerr, std::forward<TFields>(fields)...);
    std::cerr << ")";
  }

  std::cerr << '\n';
}

template <typename TResult, typename TExpected, typename... TFields>
void HandleAssertFailure(const TResult& result, const TExpected& expected,
                         const schar* prefix, const schar* message,
                         TFields&&... fields) {
  static_assert(sizeof...(fields) % 2 == 0,
                "assert fields must be provided as key/value pairs");
  std::cerr << "[ASSERT] " << prefix << ": " << message
            << " (expected=" << expected << ", actual=" << result;

  if constexpr (sizeof...(fields) > 0) {
    std::cerr << ", ";
    PrintFields(std::cerr, std::forward<TFields>(fields)...);
  }

  std::cerr << ')' << '\n';
}
}  // namespace internal

void GenerateStackTrace(schar* buffer, usize buffer_len);
}  // namespace debug
}  // namespace comet

#ifndef COMET_DEBUG

#define COMET_CASSERT(assertion, message)

#define COMET_ASSERT(assertion, prefix, message, ...) ((void)0)

#define COMET_FASSERT(expr, expected, prefix, message, ...) ((void)(expr))

#else

#define COMET_CASSERT(assertion, message) assert(assertion&& message)

#define COMET_ASSERT(assertion, prefix, message, ...)     \
  do {                                                    \
    const auto comet_is_ok{static_cast<bool>(assertion)}; \
                                                          \
    if (!comet_is_ok) {                                   \
      comet::debug::internal::PrintAssertMessage(         \
          prefix, message __VA_OPT__(, ) __VA_ARGS__);    \
      comet::debug::HandleCriticalError();                \
    }                                                     \
                                                          \
    COMET_CASSERT(comet_is_ok, "critical failure");       \
  } while (false)

#define COMET_FASSERT(expr, expected, prefix, message, ...) \
  do {                                                      \
    const auto comet_result{(expr)};                        \
    const auto comet_expected{(expected)};                  \
    const auto comet_is_ok{comet_result == comet_expected}; \
                                                            \
    if (!comet_is_ok) {                                     \
      comet::debug::internal::HandleAssertFailure(          \
          comet_result, comet_expected, prefix,             \
          message __VA_OPT__(, ) __VA_ARGS__);              \
      comet::debug::HandleCriticalError();                  \
    }                                                       \
                                                            \
    COMET_CASSERT(comet_is_ok, "critical failure");         \
  } while (false)

#endif  // !COMET_DEBUG

#endif  // COMET_COMET_CORE_DEBUG_H_