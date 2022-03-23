// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_TESTS_TESTS_ENGINE_H_
#define COMET_TESTS_TESTS_ENGINE_H_

#include "catch.hpp"
#include "comet/comet.h"
#include "comet_precompile.h"
#include "tests/dummies/dummy_object.h"

namespace comettests {
class DummyComponent final : public comet::game_object::Component {
 public:
  DummyComponent(int = 0);
  DummyComponent(const DummyComponent&);
  DummyComponent(DummyComponent&&) noexcept;
  DummyComponent& operator=(const DummyComponent&);
  DummyComponent& operator=(DummyComponent&&) noexcept;
  virtual ~DummyComponent() = default;

  virtual std::shared_ptr<Component> Clone() const override;
  int GetValue() const;

 private:
  DummyObject dummy_object_;
};
};  // namespace comettests

#endif  // COMET_TESTS_TESTS_ENGINE_H_
