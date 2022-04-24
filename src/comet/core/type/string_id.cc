// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "string_id.h"

namespace comet {
namespace stringid {
StringIdHandler::~StringIdHandler() {
  for (auto it : string_id_table) {
    free(reinterpret_cast<void*>(it.second));
  }
}

StringId StringIdHandler::Generate(const char* str, uindex length) {
  const auto string_id{utils::hash::HashCrC32(str, length)};
  const auto it{string_id_table.find(string_id)};

  if (it == string_id_table.end()) {
    string_id_table[string_id] = strdup(str);
  }

  return string_id;
}

StringId StringIdHandler::Generate(const char* str) {
  return Generate(str, std::strlen(str));
}

StringId StringIdHandler::Generate(const std::string& string) {
  return Generate(string.c_str(), string.size());
}

std::string StringIdHandler::Labelize(StringId string_id) {
  const auto it{string_id_table.find(string_id)};

  if (it == string_id_table.end()) {
    return "???";
  }

  return string_id_table[string_id];
}

StringIdHandler& GetStringIdHandler() {
  static std::unique_ptr<stringid::StringIdHandler> string_id_handler{nullptr};

  if (string_id_handler == nullptr) {
    string_id_handler = std::make_unique<stringid::StringIdHandler>();
  }

  return *string_id_handler;
}
}  // namespace stringid
}  // namespace comet
