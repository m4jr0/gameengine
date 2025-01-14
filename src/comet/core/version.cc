// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be it in the LICENSE file.

#include "comet_pch.h"

#include "version.h"

#include "comet/core/c_string.h"

namespace comet {
namespace version {
const schar* GetVersionStr() {
  constexpr auto kMaxVersionLen{31};
  static schar version[kMaxVersionLen + 1]{'\0'};

  if (version[0] != '\0') {
    return version;
  }

  usize len{0};
  ConvertToStr(kCometVersionMajor, version, kMaxVersionLen, &len);
  usize tmp{len};
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
