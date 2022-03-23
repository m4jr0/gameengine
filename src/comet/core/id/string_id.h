// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ID_STRING_ID_H_
#define COMET_COMET_CORE_ID_STRING_ID_H_

#include "comet_precompile.h"

namespace comet {
namespace core {
class StringId {
 public:
  static StringId Generate(const std::string&);
  static const std::string& GetString(const StringId&);

  StringId(const StringId&);
  StringId(StringId&&) noexcept;
  StringId& operator=(const StringId&);
  StringId& operator=(StringId&&) noexcept;
  virtual ~StringId() = default;

  bool operator==(const StringId&) const noexcept;
  bool operator!=(const StringId&) const noexcept;

  unsigned int GetHash() const noexcept;

 private:
  explicit StringId(unsigned int);

  static std::unordered_map<StringId, std::string> string_id_table_;

  unsigned int hash_ = 0;
};
}  // namespace core
}  // namespace comet

namespace std {
template <>
class hash<comet::core::StringId> {
 public:
  size_t operator()(const comet::core::StringId& string_id) const noexcept {
    return hash<unsigned int>()(string_id.GetHash());
  }
};
}  // namespace std

std::ostream& operator<<(std::ostream&, const comet::core::StringId&);

#endif  // COMET_COMET_CORE_ID_STRING_ID_H_