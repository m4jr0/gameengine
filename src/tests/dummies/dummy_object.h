// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_TESTS_DUMMY_OBJECT_H_
#define COMET_TESTS_DUMMY_OBJECT_H_

#include "comet_precompile.h"

namespace comet {
namespace comettests {
class DummyObject {
 public:
  static constexpr bool kDefaultVerbose_{false};

  DummyObject(s32 value = 0, bool is_verbose = kDefaultVerbose_);
  DummyObject(const DummyObject&);
  DummyObject(DummyObject&&) noexcept;
  DummyObject& operator=(const DummyObject&);
  DummyObject& operator=(DummyObject&&) noexcept;
  ~DummyObject();

  bool operator==(const DummyObject&) const;
  bool operator!=(const DummyObject&) const;

  std::string ToString() const;

  static uindex GetCounter() noexcept;
  uindex GetId() const noexcept;
  int GetValue() const noexcept;
  bool IsVerbose() const noexcept;
  void IsVerbose(bool) noexcept;

 private:
  void Print(const std::string& message) const;

  static uindex counter_;
  u64 id_{0};
  s32 value_{0};
  bool is_verbose_{false};
};

std::ostream& operator<<(std::ostream&, const DummyObject&);
}  // namespace comettests
}  // namespace comet

#endif  // COMET_TESTS_DUMMY_OBJECT_H_
