// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_TESTS_TESTS_ENGINE_H_
#define COMET_TESTS_TESTS_ENGINE_H_

#include "catch.hpp"
#include "comet/comet.h"
#include "comet_precompile.h"

namespace comettests {
class CometTester : public comet::core::Engine {
 public:
  CometTester() = default;
  CometTester(const CometTester&) = delete;
  CometTester(CometTester&&) = delete;
  CometTester& operator=(const CometTester&) = delete;
  CometTester& operator=(CometTester&&) = delete;
  virtual ~CometTester() = default;
};

class StopComponent final : public comet::game_object::Component {
 public:
  static const double kTimeDelta;

  StopComponent() = default;
  StopComponent(const StopComponent&);
  StopComponent(StopComponent&&) noexcept;
  StopComponent& operator=(const StopComponent&);
  StopComponent& operator=(StopComponent&&) noexcept;
  virtual ~StopComponent() = default;

  virtual std::shared_ptr<comet::game_object::Component> Clone() const override;
  void Initialize() override final;
  void Update() override final;

 private:
  double starting_time_ = 0.0;
};
};  // namespace comettests

#endif  // COMET_TESTS_TESTS_ENGINE_H_
