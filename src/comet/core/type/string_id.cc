// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "string_id.h"

#include "comet/core/c_string.h"
#include "comet/core/generator.h"
#include "comet/core/hash.h"

namespace comet {
namespace stringid {
StringIdHandler::~StringIdHandler() {
#ifdef COMET_DEBUG
  if (string_id_table.size() == 0) {
    return;
  }

  string_id_table.clear();

  if (string_id_allocator_.IsInitialized()) {
    string_id_allocator_.Destroy();
  }
#endif  // COMET_DEBUG
}

StringId StringIdHandler::Generate(const schar* str, uindex length) {
  COMET_ASSERT(str != nullptr,
               "String provided is null! Cannot generate string ID.");
  COMET_ASSERT(length > 0, "Cannot generate ID from empty string.");
  const auto string_id{HashCrC32(str, length)};

#ifdef COMET_DEBUG
  const auto it{string_id_table.find(string_id)};

  if (it == string_id_table.end()) {
    if (!string_id_allocator_.IsInitialized()) {
      string_id_allocator_.Initialize();
    }

    auto* saved_str{
        reinterpret_cast<schar*>(string_id_allocator_.Allocate(length + 1, 0))};
    Copy(saved_str, str, length);
    saved_str[length] = '\0';
    string_id_table[string_id] = saved_str;
  }
#endif  // COMET_DEBUG

  return string_id;
}

StringId StringIdHandler::Generate(const wchar* str, uindex length) {
  COMET_ASSERT(length > 0, "Length provided is 0! Cannot generate String ID !");
  return Generate(GenerateForOneFrame<schar>(str, length), length);
}

StringId StringIdHandler::Generate(const schar* str) {
  return Generate(str, GetLength(str));
}

StringId StringIdHandler::Generate(const wchar* str) {
  return Generate(str, GetLength(str));
}

// Return temporary string for debug purposes. The schar* returned SHOULD NOT be
// stored.
const schar* StringIdHandler::Labelize(StringId string_id) const {
#ifdef COMET_DEBUG
  const auto it{string_id_table.find(string_id)};

  if (it == string_id_table.end()) {
#endif  // COMET_DEBUG
    auto* label{GenerateForOneFrame<schar>(12)};
    label[0] = '?';
    ConvertToStr(string_id, label + 1, 12);
    label[11] = '?';
    label[12] = '\0';
    return label;
#ifdef COMET_DEBUG
  }

  return string_id_table.at(string_id);
#endif  // COMET_DEBUG
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
