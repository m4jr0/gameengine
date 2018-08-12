// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_UTILS_OBSERVER_HPP_
#define KOMA_UTILS_OBSERVER_HPP_

#include <string>

namespace koma {
class Observer {
public:
  virtual void ReceiveEvent(std::string) = 0;
};
};  // namespace koma

#endif  // KOMA_UTILS_OBSERVER_HPP_
