// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef TESTS_DUMMIES_HPP_
#define TESTS_DUMMIES_HPP_

#include "comet_precompile.hpp"

namespace comettests {
class DummyObject {
 public:
  static constexpr bool kDefaultVerbose_ = false;

  DummyObject(int = 0, bool = kDefaultVerbose_);
  DummyObject(const DummyObject &);
  DummyObject(DummyObject &&) noexcept;
  DummyObject &operator=(const DummyObject &);
  DummyObject &operator=(DummyObject &&) noexcept;
  virtual ~DummyObject();

  bool operator==(const DummyObject &) const;
  bool operator!=(const DummyObject &) const;

  std::string ToString() const;

  static std::size_t GetCounter() noexcept;
  std::size_t GetId() const noexcept;
  int GetValue() const noexcept;
  bool IsVerbose() const noexcept;
  void IsVerbose(bool) noexcept;

 private:
  void Print(const std::string &) const;

  static std::size_t counter_;
  std::size_t id_ = 0;
  int value_ = 0;
  bool is_verbose_ = false;
};

std::ostream &operator<<(std::ostream &, const comettests::DummyObject &);
}  // namespace comettests

#endif  // TESTS_DUMMIES_HPP_
