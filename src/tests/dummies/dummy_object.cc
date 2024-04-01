// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "dummy_object.h"

namespace comet {
namespace comettests {
usize DummyObject::counter_{0};

DummyObject::DummyObject(s32 value) : id_{counter_++}, value_{value} {}

DummyObject::DummyObject(const DummyObject& other)
    : id_{counter_++}, value_{other.value_} {}

DummyObject::DummyObject(DummyObject&& other) noexcept
    : id_{other.id_}, value_{other.value_} {
  other.id_ = 0;
  other.value_ = 0;
}

DummyObject& DummyObject::operator=(const DummyObject& other) {
  if (this == &other) {
    return *this;
  }

  id_ = other.id_;
  value_ = other.value_;
  return *this;
}

DummyObject& DummyObject::operator=(DummyObject&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  id_ = other.id_;
  value_ = other.value_;
  other.id_ = 0;
  other.value_ = 0;
  return *this;
}

bool DummyObject::operator==(const DummyObject& other) const {
  return value_ == other.value_;
}

bool DummyObject::operator!=(const DummyObject& other) const {
  return value_ != other.value_;
}

usize DummyObject::GetCounter() noexcept { return counter_; }

u64 DummyObject::GetId() const noexcept { return id_; }

s32 DummyObject::GetValue() const noexcept { return value_; }
}  // namespace comettests
}  // namespace comet
