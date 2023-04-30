// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "configuration_manager.h"

#include "comet/core/file_system.h"

namespace comet {
namespace conf {
ConfigurationManager::ConfigurationManager(
    const ConfigurationManagerDescr& descr)
    : Manager{descr} {}

void ConfigurationManager::Initialize() {
  Manager::Initialize();
  ParseConfFile();
}

void ConfigurationManager::Shutdown() {
  values_.clear();
  Manager::Shutdown();
}

void ConfigurationManager::ParseConfFile() {
  std::ifstream in_file;

  // kConfigFilePath_ is const, so using .data() is okay.
  if (!OpenBinaryFileToReadFrom(kConfigFilePath_.data(), in_file)) {
    COMET_LOG_CORE_INFO("No configuration file at: ", kConfigFilePath_, ".");
    return;
  }

  if (!in_file.good()) {
    COMET_LOG_CORE_ERROR("Invalid configuration file at: ", kConfigFilePath_,
                         ". Ignoring.");
    return;
  }

  std::string line;

  while (in_file.good() && std::getline(in_file, line)) {
    Trim(line);

    if (line.empty() || line[0] == '#') {
      continue;
    }

    const auto key_val_delimiter_pos{line.find('=')};

    if (key_val_delimiter_pos == std::string::npos) {
      continue;
    }

    auto raw_value{line.substr(key_val_delimiter_pos + 1)};
    const auto comment_delimiter_pos{raw_value.find('#')};

    if (comment_delimiter_pos == std::string::npos) {
      ParseKeyValuePair(line.substr(0, key_val_delimiter_pos),
                        std::move(raw_value));
    } else {
      ParseKeyValuePair(line.substr(0, key_val_delimiter_pos),
                        raw_value.substr(0, comment_delimiter_pos));
    }
  }
}

ConfValue& ConfigurationManager::Get(ConfKey key) {
  auto it{values_.find(key)};

  if (it == values_.end()) {
    return values_.emplace(key, GetDefaultValue(key)).first->second;
  }

  return it->second;
}

ConfValue ConfigurationManager::Get(ConfKey key) const {
  const auto it{values_.find(key)};

  if (it == values_.end()) {
    return GetDefaultValue(key);
  }

  return it->second;
}

std::string ConfigurationManager::GetStr(ConfKey key) const {
  return std::string{Get(key).str};
}

u8 ConfigurationManager::GetU8(ConfKey key) const { return Get(key).ubyte; }

u16 ConfigurationManager::GetU16(ConfKey key) const { return Get(key).ushort; }

u32 ConfigurationManager::GetU32(ConfKey key) const { return Get(key).u; }

u64 ConfigurationManager::GetU64(ConfKey key) const { return Get(key).ulong; }

s8 ConfigurationManager::GetS8(ConfKey key) const { return Get(key).sbyte; }

s16 ConfigurationManager::GetS16(ConfKey key) const { return Get(key).sshort; }

s32 ConfigurationManager::GetS32(ConfKey key) const { return Get(key).s; }

s64 ConfigurationManager::GetS64(ConfKey key) const { return Get(key).slong; }

f32 ConfigurationManager::GetF32(ConfKey key) const { return Get(key).f; }

f64 ConfigurationManager::GetF64(ConfKey key) const { return Get(key).flong; }

uindex ConfigurationManager::GetIndex(ConfKey key) const {
  return Get(key).index;
}

ux ConfigurationManager::GetUx(ConfKey key) const { return Get(key).uarch; }

sx ConfigurationManager::GetSx(ConfKey key) const { return Get(key).sarch; }

fx ConfigurationManager::GetFx(ConfKey key) const { return Get(key).farch; }

bool ConfigurationManager::GetBool(ConfKey key) const { return Get(key).flag; }

void ConfigurationManager::Set(ConfKey key, const ConfValue& value) {
  values_.emplace(key, value);
}

void ConfigurationManager::SetStr(ConfKey key, const std::string& value) {
  return SetStr(key, value.c_str(), value.size());
}

void ConfigurationManager::SetStr(ConfKey key, const schar* value) {
  return SetStr(key, value, strlen(value));
}

void ConfigurationManager::SetStr(ConfKey key, const schar* value,
                                  uindex length) {
  if (length > kMaxStrValueLength) {
    COMET_LOG_CORE_ERROR("String value for key ", COMET_STRING_ID_LABEL(key),
                         "is too long. Length provided is ", length,
                         ". Max allowed is ", kMaxStrValueLength,
                         ". Value will be trimmed.");
    length = kMaxStrValueLength;
  }

  std::memcpy(Get(key).str, value, length);
}

void ConfigurationManager::SetU8(ConfKey key, u8 value) {
  Get(key).ubyte = value;
}

void ConfigurationManager::SetU16(ConfKey key, u16 value) {
  Get(key).ushort = value;
}

void ConfigurationManager::SetU32(ConfKey key, u32 value) {
  Get(key).u = value;
}

void ConfigurationManager::SetU64(ConfKey key, u64 value) {
  Get(key).ulong = value;
}

void ConfigurationManager::SetS8(ConfKey key, s8 value) {
  Get(key).sbyte = value;
}

void ConfigurationManager::SetS16(ConfKey key, s16 value) {
  Get(key).sshort = value;
}

void ConfigurationManager::SetS32(ConfKey key, s32 value) {
  Get(key).s = value;
}

void ConfigurationManager::SetS64(ConfKey key, s64 value) {
  Get(key).slong = value;
}

void ConfigurationManager::SetF32(ConfKey key, f32 value) {
  Get(key).f = value;
}

void ConfigurationManager::SetF64(ConfKey key, f64 value) {
  Get(key).flong = value;
}

void ConfigurationManager::SetIndex(ConfKey key, uindex value) {
  Get(key).index = value;
}

void ConfigurationManager::SetUx(ConfKey key, ux value) {
  Get(key).uarch = value;
}

void ConfigurationManager::SetSx(ConfKey key, sx value) {
  Get(key).sarch = value;
}

void ConfigurationManager::SetFx(ConfKey key, fx value) {
  Get(key).farch = value;
}

void ConfigurationManager::SetBool(ConfKey key, bool value) {
  Get(key).flag = value;
}
}  // namespace conf
}  // namespace comet
