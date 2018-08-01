// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef TESTS_GAME_HPP_
#define TESTS_GAME_HPP_

#include "../koma/core/game.hpp"

#include "catch.hpp"

namespace komatests {
class StopComponent final : public koma::Component {
 public:
  static const double kTimeDelta;

  void Initialize() override final;
  void Update(double) override final;

 private:
  double starting_time_ = 0.0;
};
};  // namespace komatests

#endif  // TESTS_GAME_HPP_
