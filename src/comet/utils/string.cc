// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "string.h"

namespace comet {
namespace utils {
namespace string {
uindex GetSubStrNthPos(const std::string& str, const char* to_search,
                       uindex count) {
  if (str.size() == 0 || count == 0) {
    return kInvalidIndex;
  }

  uindex occurrence_counter{0};
  uindex nth_pos{str.size()};

  while (nth_pos != std::string::npos && occurrence_counter < count) {
    nth_pos = str.find_last_of(to_search, nth_pos - 1);
    ++occurrence_counter;
  }

  if (nth_pos == std::string::npos || occurrence_counter != count) {
    return kInvalidIndex;
  }

  return nth_pos;
}

uindex GetSubStrNthPos(const std::string& str, const std::string& to_search,
                       uindex count) {
  return GetSubStrNthPos(str, to_search.c_str(), count);
}
}  // namespace string
}  // namespace utils
}  // namespace comet
