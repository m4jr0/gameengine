// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_LOCATOR_HPP_
#define KOMA_CORE_LOCATOR_HPP_

#include "game.hpp"
#include "input_manager.hpp"
#include "time_manager.hpp"

namespace koma {
class Locator {
 public:
  static void Initialize();

  static InputManager& input_manager() { return *Locator::input_manager_; }
  static TimeManager& time_manager() { return *Locator::time_manager_; }

  static void input_manager(InputManager* input_manager);

 private:
  static InputManager* input_manager_;
  static NullInputManager null_input_manager_;
  static TimeManager* time_manager_;

  static void time_manager(TimeManager* time_manager);

  friend class Game;
};
};  // namespace koma

#endif  // KOMA_CORE_LOCATOR_HPP_