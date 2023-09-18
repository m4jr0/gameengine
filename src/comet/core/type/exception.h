// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_EXCEPTION_H_
#define COMET_COMET_CORE_TYPE_EXCEPTION_H_

#include "comet_precompile.h"

namespace comet {
class maximum_capacity_reached_error : public std::runtime_error {
 public:
  maximum_capacity_reached_error(uindex capacity)
      : runtime_error("Structure instance is full (" +
                      std::to_string(capacity) + ")") {}
};

class empty_error : public std::runtime_error {
 public:
  empty_error() : runtime_error("Structure instance is empty") {}
};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_EXCEPTION_H_
