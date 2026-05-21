// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_CONF_CONF_DEFAULTS_H_
#define COMET_RUNTIME_CONF_CONF_DEFAULTS_H_

#include "comet/core/essentials.h"

namespace comet {
namespace conf {
ConfValue GetDefaultValue(const schar* key);
ConfValue GetDefaultValue(ConfKey key);
}  // namespace conf
}  // namespace comet

#endif  // COMET_RUNTIME_CONF_CONF_DEFAULTS_H_