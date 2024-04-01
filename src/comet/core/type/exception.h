// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_EXCEPTION_H_
#define COMET_COMET_CORE_TYPE_EXCEPTION_H_

#include "comet/core/essentials.h"

namespace comet {
class MaximumCapacityReachedError : public std::runtime_error {
 public:
  MaximumCapacityReachedError(usize capacity)
      : runtime_error(GenerateTmpErrorMessage(capacity)) {}

 private:
  static const schar* GenerateTmpErrorMessage(usize capacity);
};

class EmptyError : public std::runtime_error {
 public:
  EmptyError() : runtime_error("Structure instance is empty") {}
};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_EXCEPTION_H_
