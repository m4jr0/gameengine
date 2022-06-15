// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_UTILS_STRING_H_
#define COMET_COMET_UTILS_STRING_H_

#include "comet_precompile.h"

namespace comet {
namespace utils {
namespace string {
uindex GetSubStrNthPos(const std::string& str, const char* to_search,
                       uindex count);
uindex GetSubStrNthPos(const std::string& str, const std::string& to_search,
                       uindex count);
}  // namespace string
}  // namespace utils
}  // namespace comet

#endif  // COMET_COMET_UTILS_STRING_H_
