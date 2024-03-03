// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_TESTS_DUMMY_OBJECT_H_
#define COMET_TESTS_DUMMY_OBJECT_H_

#include "comet_precompile.h"

namespace comet {
namespace comettests {
class DummyObject {
 public:
  DummyObject(s32 value = 0);
  DummyObject(const DummyObject&);
  DummyObject(DummyObject&&) noexcept;
  DummyObject& operator=(const DummyObject&);
  DummyObject& operator=(DummyObject&&) noexcept;
  ~DummyObject() = default;

  bool operator==(const DummyObject&) const;
  bool operator!=(const DummyObject&) const;

  static uindex GetCounter() noexcept;
  uindex GetId() const noexcept;
  s32 GetValue() const noexcept;

 private:
  static uindex counter_;
  u64 id_{0};
  s32 value_{0};
};
}  // namespace comettests
}  // namespace comet

#endif  // COMET_TESTS_DUMMY_OBJECT_H_
