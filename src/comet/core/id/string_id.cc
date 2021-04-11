// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "string_id.h"

#include "comet/utils/hash.h"

namespace comet {
namespace core {
std::unordered_map<StringId, std::string> StringId::string_id_table_;

const std::string& StringId::GetString(const StringId& string_id) {
  return string_id_table_[string_id];
}

StringId::StringId(const StringId& other) : hash_(other.hash_) {}

StringId::StringId(StringId&& other) noexcept : hash_(std::move(other.hash_)) {}

StringId& StringId::operator=(const StringId& other) {
  if (this == &other) {
    return *this;
  }

  hash_ = other.hash_;
  return *this;
}

StringId& StringId::operator=(StringId&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  hash_ = std::move(other.hash_);
  return *this;
}

bool StringId::operator==(const StringId& other) const noexcept {
  return hash_ == other.hash_;
}

bool StringId::operator!=(const StringId& other) const noexcept {
  return !StringId::operator==(other);
}

unsigned int StringId::GetHash() const noexcept { return hash_; }

StringId StringId::Generate(const std::string& string) {
  const auto string_id = StringId(utils::hash::HashCrC32(string));
  const auto it = string_id_table_.find(string_id);

  if (it == string_id_table_.end()) {
    string_id_table_[string_id] = string;
  }

  return string_id;
}

StringId::StringId(unsigned int hash) : hash_(hash) {}
}  // namespace core
}  // namespace comet

std::ostream& operator<<(std::ostream& stream,
                         const comet::core::StringId& string_id) {
  stream << "StringId(" << comet::core::StringId::GetString(string_id) << "|"
         << string_id.GetHash() << ")";
  return stream;
}
