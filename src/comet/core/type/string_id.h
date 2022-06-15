// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_STRING_ID_H_
#define COMET_COMET_CORE_TYPE_STRING_ID_H_

#include "comet_precompile.h"

#include "comet/utils/hash.h"

namespace comet {
namespace stringid {
using StringId = u32;

class StringIdHandler {
 public:
  StringIdHandler() = default;
  StringIdHandler(const StringIdHandler&) = delete;
  StringIdHandler(StringIdHandler&&) = delete;
  StringIdHandler& operator=(const StringIdHandler&) = delete;
  StringIdHandler& operator=(StringIdHandler&&) = delete;
  ~StringIdHandler();

  StringId Generate(const char* string, uindex length);
  StringId Generate(const char* string);
  StringId Generate(const std::string& string);
  std::string Labelize(StringId string_id);

 private:
  std::unordered_map<StringId, char*> string_id_table;
};

extern StringIdHandler* SetHandler(bool is_destroy = false);
}  // namespace stringid
}  // namespace comet

#define COMET_STRING_ID(string) comet::stringid::SetHandler()->Generate(string)
#define COMET_STRING_ID_LABEL(string_id) \
  comet::stringid::SetHandler()->Labelize(string_id)
#define COMET_STRING_ID_DESTROY() comet::stringid::SetHandler(true)

#endif  // COMET_COMET_CORE_TYPE_STRING_ID_H_