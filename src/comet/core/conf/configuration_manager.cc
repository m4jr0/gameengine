// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "configuration_manager.h"

#include "comet/core/file_system.h"

namespace comet {
namespace conf {
ConfigurationManager& ConfigurationManager::Get() {
  static ConfigurationManager singleton{};
  return singleton;
}

void ConfigurationManager::Initialize() {
  Manager::Initialize();

  values_.insert({kApplicationName, GetDefaultValue(kApplicationName)});

  values_.insert(
      {kApplicationMajorVersion, GetDefaultValue(kApplicationMajorVersion)});
  values_.insert(
      {kApplicationMinorVersion, GetDefaultValue(kApplicationMinorVersion)});
  values_.insert(
      {kApplicationPatchVersion, GetDefaultValue(kApplicationPatchVersion)});
  values_.insert({kCoreMsPerUpdate, GetDefaultValue(kCoreMsPerUpdate)});
  values_.insert({kCoreOneFrameAllocatorCapacity,
                  GetDefaultValue(kCoreOneFrameAllocatorCapacity)});
  values_.insert({kCoreTwoFrameAllocatorCapacity,
                  GetDefaultValue(kCoreTwoFrameAllocatorCapacity)});
  values_.insert({kCoreTStringAllocatorCapacity,
                  GetDefaultValue(kCoreTStringAllocatorCapacity)});
  values_.insert({kEventMaxQueueSize, GetDefaultValue(kEventMaxQueueSize)});
  values_.insert({kRenderingDriver, GetDefaultValue(kRenderingDriver)});
  values_.insert(
      {kRenderingWindowWidth, GetDefaultValue(kRenderingWindowWidth)});
  values_.insert(
      {kRenderingWindowHeight, GetDefaultValue(kRenderingWindowHeight)});
  values_.insert(
      {kRenderingClearColorR, GetDefaultValue(kRenderingClearColorR)});
  values_.insert(
      {kRenderingClearColorG, GetDefaultValue(kRenderingClearColorG)});
  values_.insert(
      {kRenderingClearColorB, GetDefaultValue(kRenderingClearColorB)});
  values_.insert(
      {kRenderingClearColorA, GetDefaultValue(kRenderingClearColorA)});
  values_.insert({kRenderingIsVsync, GetDefaultValue(kRenderingIsVsync)});
  values_.insert({kRenderingIsTripleBuffering,
                  GetDefaultValue(kRenderingIsTripleBuffering)});
  values_.insert({kRenderingFpsCap, GetDefaultValue(kRenderingFpsCap)});
  values_.insert(
      {kRenderingAntiAliasing, GetDefaultValue(kRenderingAntiAliasing)});
  values_.insert({kRenderingIsSamplerAnisotropy,
                  GetDefaultValue(kRenderingIsSamplerAnisotropy)});
  values_.insert({kRenderingIsSampleRateShading,
                  GetDefaultValue(kRenderingIsSampleRateShading)});
  values_.insert({kRenderingOpenGlMajorVersion,
                  GetDefaultValue(kRenderingOpenGlMajorVersion)});
  values_.insert({kRenderingOpenGlMinorVersion,
                  GetDefaultValue(kRenderingOpenGlMinorVersion)});
  values_.insert({kRenderingVulkanVariantVersion,
                  GetDefaultValue(kRenderingVulkanVariantVersion)});
  values_.insert({kRenderingVulkanMajorVersion,
                  GetDefaultValue(kRenderingVulkanMajorVersion)});
  values_.insert({kRenderingVulkanMinorVersion,
                  GetDefaultValue(kRenderingVulkanMinorVersion)});
  values_.insert({kRenderingVulkanPatchVersion,
                  GetDefaultValue(kRenderingVulkanPatchVersion)});
  values_.insert({kRenderingVulkanMaxFramesInFlight,
                  GetDefaultValue(kRenderingVulkanMaxFramesInFlight)});
  values_.insert({kResourceRootPath, GetDefaultValue(kResourceRootPath)});

  ParseConfFile();
}

void ConfigurationManager::Shutdown() {
  values_.clear();
  Manager::Shutdown();
}

void ConfigurationManager::ParseConfFile() {
  std::ifstream in_file;

  if (!OpenFileToReadFrom(kConfigFileRelativePath_, in_file)) {
    COMET_LOG_CORE_INFO("No configuration file at: ", kConfigFileRelativePath_,
                        ".");
    return;
  }

  if (!in_file.good()) {
    COMET_LOG_CORE_ERROR("Invalid configuration file at: ",
                         kConfigFileRelativePath_, ". Ignoring.");
    return;
  }

  schar line[kMaxLineLength];
  uindex line_len;

  while (in_file.good()) {
    if (!GetLine(in_file, line, kMaxLineLength, &line_len)) {
      continue;
    }

    Trim(line, line_len, &line_len);

    if (IsEmpty(line, line_len) || line[0] == '#') {
      continue;
    }

    auto key_val_delimiter_pos{GetIndexOf(line, '=', line_len)};

    if (key_val_delimiter_pos == kInvalidIndex) {
      continue;
    }

    auto raw_key_len{key_val_delimiter_pos};
    schar raw_key[kMaxKeyLength];
    GetSubString(raw_key, line, line_len, 0, key_val_delimiter_pos);
    Trim(raw_key, raw_key_len, &raw_key_len);

    auto raw_value_len{line_len - key_val_delimiter_pos - 1};
    schar raw_value[kMaxValueLength];
    GetSubString(raw_value, line, line_len, key_val_delimiter_pos + 1);
    Trim(raw_value, raw_value_len, &raw_value_len);

    const auto comment_delimiter_pos{GetIndexOf(raw_value, '#', raw_value_len)};

    if (comment_delimiter_pos != kInvalidIndex) {
      GetSubString(raw_value, raw_value, raw_value_len, 0,
                   comment_delimiter_pos);
    }

    if (raw_key_len == 0 || raw_value_len == 0) {
      continue;
    }

    ParseKeyValuePair(raw_key, raw_key_len, raw_value, raw_value_len);
  }
}

ConfValue& ConfigurationManager::Get(ConfKey key) {
  auto it{values_.find(key)};
  COMET_ASSERT(it != values_.end(),
               "Unknown configuration key: ", COMET_STRING_ID_LABEL(key), "!");
  return it->second;
}

const ConfValue& ConfigurationManager::Get(ConfKey key) const {
  const auto it{values_.find(key)};
  COMET_ASSERT(it != values_.end(),
               "Unknown configuration key: ", COMET_STRING_ID_LABEL(key), "!");
  return it->second;
}

const schar* ConfigurationManager::GetStr(ConfKey key) const {
  return Get(key).str_value;
}

const tchar* ConfigurationManager::GetTStr(ConfKey key) const {
#ifdef COMET_WIDE_TCHAR
  return Get(key).wstr_value;
#else
  return Get(key).str_value;
#endif  // COMET_WIDE_TCHAR
}

void ConfigurationManager::GetTStr(ConfKey key, TString& str) const {
#ifdef COMET_WIDE_TCHAR
  str = Get(key).wstr_value;
#else
  str = Get(key).str_value;
#endif  // COMET_WIDE_TCHAR
}

u8 ConfigurationManager::GetU8(ConfKey key) const { return Get(key).u8_value; }

u16 ConfigurationManager::GetU16(ConfKey key) const {
  return Get(key).u16_value;
}

u32 ConfigurationManager::GetU32(ConfKey key) const {
  return Get(key).u32_value;
}

u64 ConfigurationManager::GetU64(ConfKey key) const {
  return Get(key).u64_value;
}

s8 ConfigurationManager::GetS8(ConfKey key) const { return Get(key).s8_value; }

s16 ConfigurationManager::GetS16(ConfKey key) const {
  return Get(key).s16_value;
}

s32 ConfigurationManager::GetS32(ConfKey key) const {
  return Get(key).s32_value;
}

s64 ConfigurationManager::GetS64(ConfKey key) const {
  return Get(key).s64_value;
}

f32 ConfigurationManager::GetF32(ConfKey key) const {
  return Get(key).f32_value;
}

f64 ConfigurationManager::GetF64(ConfKey key) const {
  return Get(key).f64_value;
}

uindex ConfigurationManager::GetIndex(ConfKey key) const {
  return Get(key).uindex_value;
}

ux ConfigurationManager::GetUx(ConfKey key) const { return Get(key).ux_value; }

sx ConfigurationManager::GetSx(ConfKey key) const { return Get(key).sx_value; }

fx ConfigurationManager::GetFx(ConfKey key) const { return Get(key).fx_value; }

bool ConfigurationManager::GetBool(ConfKey key) const {
  return Get(key).bool_value;
}

void ConfigurationManager::Set(ConfKey key, const ConfValue& value) {
  values_.emplace(key, value);
}

void ConfigurationManager::SetStr(ConfKey key, const schar* value) {
  return SetStr(key, value, GetLength(value));
}

void ConfigurationManager::SetStr(ConfKey key, const schar* value,
                                  uindex length) {
  if (length > kMaxStrValueLength) {
    COMET_LOG_CORE_ERROR("String value for key ", COMET_STRING_ID_LABEL(key),
                         "is too long. Length provided is ", length,
                         ". Max allowed is ", kMaxStrValueLength,
                         ". Value will be trimmed.");
    length = kMaxStrValueLength - 1;
  }

  auto* str{Get(key).str_value};
  Copy(str, value, length);
  str[length] = '\0';
}

void ConfigurationManager::SetTStr(ConfKey key, const tchar* value) {
  return SetTStr(key, value, GetLength(value));
}

void ConfigurationManager::SetTStr(ConfKey key, const tchar* value,
                                   uindex length) {
  if (length > kMaxStrValueLength) {
    COMET_LOG_CORE_ERROR("TString value for key ", COMET_STRING_ID_LABEL(key),
                         "is too long. Length provided is ", length,
                         ". Max allowed is ", kMaxStrValueLength,
                         ". Value will be trimmed.");
    length = kMaxStrValueLength - 1;
  }

#ifdef COMET_WIDE_TCHAR
  auto* str{Get(key).wstr_value};
#else
  auto* str{Get(key).str_value};
#endif  // COMET_WIDE_TCHAR

  Copy(str, value, length);
  str[length] = COMET_TCHAR('\0');
}

void ConfigurationManager::SetU8(ConfKey key, u8 value) {
  Get(key).u8_value = value;
}

void ConfigurationManager::SetU16(ConfKey key, u16 value) {
  Get(key).u16_value = value;
}

void ConfigurationManager::SetU32(ConfKey key, u32 value) {
  Get(key).u32_value = value;
}

void ConfigurationManager::SetU64(ConfKey key, u64 value) {
  Get(key).u64_value = value;
}

void ConfigurationManager::SetS8(ConfKey key, s8 value) {
  Get(key).s8_value = value;
}

void ConfigurationManager::SetS16(ConfKey key, s16 value) {
  Get(key).s16_value = value;
}

void ConfigurationManager::SetS32(ConfKey key, s32 value) {
  Get(key).s32_value = value;
}

void ConfigurationManager::SetS64(ConfKey key, s64 value) {
  Get(key).s64_value = value;
}

void ConfigurationManager::SetF32(ConfKey key, f32 value) {
  Get(key).f32_value = value;
}

void ConfigurationManager::SetF64(ConfKey key, f64 value) {
  Get(key).f64_value = value;
}

void ConfigurationManager::SetIndex(ConfKey key, uindex value) {
  Get(key).uindex_value = value;
}

void ConfigurationManager::SetUx(ConfKey key, ux value) {
  Get(key).ux_value = value;
}

void ConfigurationManager::SetSx(ConfKey key, sx value) {
  Get(key).sx_value = value;
}

void ConfigurationManager::SetFx(ConfKey key, fx value) {
  Get(key).fx_value = value;
}

void ConfigurationManager::SetBool(ConfKey key, bool value) {
  Get(key).bool_value = value;
}

void ConfigurationManager::ParseKeyValuePair(schar* raw_key,
                                             uindex key_val_delimiter_pos,
                                             schar* value, uindex value_len) {
  auto key{COMET_STRING_ID(Trim(raw_key, key_val_delimiter_pos))};
  Trim(value, value_len);

  if (key == kApplicationName || key == kRenderingDriver ||
      key == kRenderingAntiAliasing) {
    SetStr(key, value);
  } else if (key == kEventMaxQueueSize || key == kApplicationMajorVersion ||
             key == kApplicationMinorVersion ||
             key == kApplicationPatchVersion || key == kRenderingWindowWidth ||
             key == kRenderingWindowHeight || key == kRenderingFpsCap ||
             key == kRenderingOpenGlMajorVersion ||
             key == kRenderingOpenGlMinorVersion ||
             key == kRenderingVulkanVariantVersion ||
             key == kRenderingVulkanMajorVersion ||
             key == kRenderingVulkanMinorVersion ||
             key == kRenderingVulkanPatchVersion ||
             key == kRenderingVulkanMaxFramesInFlight) {
    SetU16(key, ParseU16(value));
  } else if (key == kCoreOneFrameAllocatorCapacity ||
             key == kCoreTwoFrameAllocatorCapacity ||
             key == kCoreTStringAllocatorCapacity) {
    SetU32(key, ParseU32(value));
  } else if (key == kRenderingClearColorR || key == kRenderingClearColorG ||
             key == kRenderingClearColorB || key == kRenderingClearColorA) {
    SetF32(key, ParseF32(value));
  } else if (key == kCoreMsPerUpdate) {
    SetF64(key, ParseF64(value));
  } else if (key == kRenderingIsVsync || key == kRenderingIsTripleBuffering ||
             key == kRenderingIsSamplerAnisotropy ||
             key == kRenderingIsSampleRateShading) {
    SetBool(key, ParseBool(value));
  } else if (key == kResourceRootPath) {
#ifdef COMET_WIDE_TCHAR
    wchar path[kMaxStrValueLength];
    Copy(path, value, value_len);
#else
    auto* path{value};
#endif  // COMET_WIDE_TCHAR

    SetTStr(key, path, value_len);
  } else {
    COMET_LOG_CORE_WARNING("Invalid configuration key: \"",
                           COMET_STRING_ID_LABEL(key), "\". Ignoring.");
  }
}
}  // namespace conf
}  // namespace comet
