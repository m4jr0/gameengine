// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "string_id.h"

namespace comet {
namespace stringid {
StringIdHandler::~StringIdHandler() {
  if (string_id_table.size() == 0) {
    return;
  }

  for (auto& it : string_id_table) {
    if (it.second == nullptr) {
      continue;
    }

    delete it.second;
  }

  string_id_table.clear();
}

StringId StringIdHandler::Generate(const char* str, uindex length) {
  COMET_ASSERT(str != nullptr,
               "String provided is null! Cannot generate string ID.");
  COMET_ASSERT(length > 0, "Cannot generate ID from empty string.");
  const auto string_id{utils::hash::HashCrC32(str, length)};
  const auto it{string_id_table.find(string_id)};

  if (it == string_id_table.end()) {
    string_id_table[string_id] = strdup(str);
  }

  return string_id;
}

StringId StringIdHandler::Generate(const char* str) {
  COMET_ASSERT(str != nullptr,
               "String provided is null! Cannot generate string ID.");
  return Generate(str, std::strlen(str));
}

StringId StringIdHandler::Generate(const std::string& string) {
  COMET_ASSERT(string.length() > 0, "Cannot generate ID from empty string.");
  return Generate(string.c_str(), string.size());
}

std::string StringIdHandler::Labelize(StringId string_id) {
  const auto it{string_id_table.find(string_id)};

  if (it == string_id_table.end()) {
    return "???";
  }

  return string_id_table[string_id];
}

StringIdHandler* SetHandler(bool is_destroy) {
  static std::unique_ptr<stringid::StringIdHandler> string_id_handler{nullptr};

  if (is_destroy) {
    string_id_handler = nullptr;
    return string_id_handler.get();
  }

  if (string_id_handler == nullptr) {
    string_id_handler = std::make_unique<stringid::StringIdHandler>();
  }

  return string_id_handler.get();
}
}  // namespace stringid
}  // namespace comet
