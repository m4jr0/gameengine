// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be it in the LICENSE file.

#include "version.h"

namespace comet {
namespace version {
const schar* GetVersionStr() {
  constexpr auto kMaxVersionLen{31};
  static schar version[kMaxVersionLen + 1]{'\0'};

  if (version[0] != '\0') {
    return version;
  }

  uindex len{0};
  ConvertToStr(kCometVersionMajor, version, kMaxVersionLen, &len);
  uindex tmp{len};
  Copy(version, ".", 1, len++);
  ConvertToStr(kCometVersionMinor, version + len, kMaxVersionLen - len, &tmp);
  len += tmp;
  Copy(version, ".", 1, len++);
  ConvertToStr(kCometVersionPatch, version + len, kMaxVersionLen - len, &tmp);
  len += tmp;
  version[len] = '\0';
  return version;
}
}  // namespace version
}  // namespace comet
