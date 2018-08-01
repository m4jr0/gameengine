// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_INPUT_MANAGER_HPP_
#define KOMA_CORE_INPUT_MANAGER_HPP_

namespace koma {
enum class Input {
  kToBeImplemented
};

class InputManager {
 public:
  virtual bool GetInput(Input) = 0;
};

class NullInputManager : public InputManager {
 public:
   virtual bool GetInput(Input) override { return false;  };
};
};  // namespace koma

#endif  // KOMA_CORE_INPUT_MANAGER_HPP_
