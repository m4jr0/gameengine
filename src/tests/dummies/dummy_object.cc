// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "dummy_object.h"

#include "catch.hpp"

namespace comettests {
std::size_t DummyObject::counter_ = 0;

DummyObject::DummyObject(int value, bool is_verbose)
    : id_(counter_++), value_(value), is_verbose_(is_verbose) {
  if (is_verbose_) {
    Print("(int value, bool is_verbose) Constructor");
  }
}

DummyObject::DummyObject(const DummyObject& other)
    : id_(counter_++), value_(other.value_) {
  if (is_verbose_) {
    Print("Copy constructor");
  }
}

DummyObject::DummyObject(DummyObject&& other) noexcept
    : id_(std::move(other.id_)), value_(std::move(other.value_)) {
  if (is_verbose_) {
    Print("Move constructor");
  }
}

DummyObject& DummyObject::operator=(const DummyObject& other) {
  if (is_verbose_) {
    other.Print("operator=");
  }

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

  id_ = std::move(other.id_);
  value_ = std::move(other.value_);
  return *this;
}

DummyObject::~DummyObject() {
  if (is_verbose_) {
    Print("Destructor");
  }
}

bool DummyObject::operator==(const DummyObject& other) const {
  return value_ == other.value_;
}

bool DummyObject::operator!=(const DummyObject& other) const {
  return value_ != other.value_;
}

std::string DummyObject::ToString() const {
  return "DummyObject#" + std::to_string(id_) + "(" + std::to_string(value_) +
         ")";
}

std::size_t DummyObject::GetCounter() noexcept { return counter_; }

std::size_t DummyObject::GetId() const noexcept { return id_; }

int DummyObject::GetValue() const noexcept { return value_; }

bool DummyObject::IsVerbose() const noexcept { return false; }

void DummyObject::IsVerbose(bool is_verbose) noexcept {
  is_verbose_ = is_verbose;
}

void DummyObject::Print(const std::string& message) const {
  std::cout << ToString() << ": " << message << std::endl;
}

std::ostream& operator<<(std::ostream& out,
                         const comettests::DummyObject& dummy_object) {
  return out << dummy_object.ToString();
}
}  // namespace comettests
