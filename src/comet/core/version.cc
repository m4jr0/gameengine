// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be it in the LICENSE file.

#include "version.h"

#include "comet/utils/string.h"

namespace comet {
namespace version {
std::string GetFormattedVersion() {
  // Because of SSO, no memory allocation should happen.
  auto version_major{std::to_string(kCometVersionMajor)};
  auto version_minor{std::to_string(kCometVersionMinor)};
  auto version_patch{std::to_string(kCometVersionPatch)};

  std::string version{};
  version.reserve(version_major.size() + version_minor.size() +
                  version_patch.size() + 2);

  // In-place allocations.
  version += version_major;
  version += '.';
  version += version_minor;
  version += '.';
  version += version_patch;
  return version;
}
}  // namespace version
}  // namespace comet
