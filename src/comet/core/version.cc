// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be it in the LICENSE file.

#include "version.h"

namespace comet {
namespace version {
std::string GetFormattedVersion() {
  return std::to_string(kCometVersionMajor) + "." +
         std::to_string(kCometVersionMinor) + "." +
         std::to_string(kCometVersionPatch);
}
}  // namespace version
}  // namespace comet
