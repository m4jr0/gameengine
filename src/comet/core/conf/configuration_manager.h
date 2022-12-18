// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONF_CONFIGURATION_MANAGER_H_
#define COMET_COMET_CORE_CONF_CONFIGURATION_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/conf/configuration_value.h"
#include "comet/utils/string.h"

namespace comet {
namespace conf {
class ConfigurationManager {
 public:
  ConfigurationManager() = default;
  ConfigurationManager(const ConfigurationManager&) = delete;
  ConfigurationManager(ConfigurationManager&&) = delete;
  ConfigurationManager& operator=(const ConfigurationManager&) = delete;
  ConfigurationManager& operator=(ConfigurationManager&&) = delete;
  ~ConfigurationManager() = default;

  void Initialize();
  void Destroy();
  void ParseConfFile();

  ConfValue& Get(ConfKey key);
  std::string GetStr(ConfKey key);
  u8 GetU8(ConfKey key);
  u16 GetU16(ConfKey key);
  u32 GetU32(ConfKey key);
  u64 GetU64(ConfKey key);
  s8 GetS8(ConfKey key);
  s16 GetS16(ConfKey key);
  s32 GetS32(ConfKey key);
  s64 GetS64(ConfKey key);
  f32 GetF32(ConfKey key);
  f64 GetF64(ConfKey key);
  uindex GetIndex(ConfKey key);
  ux GetUx(ConfKey key);
  sx GetSx(ConfKey key);
  fx GetFx(ConfKey key);
  bool GetBool(ConfKey key);

  void Set(ConfKey key, ConfValue value);

  void SetStr(ConfKey key, const std::string& value);
  void SetStr(ConfKey key, const char* value);
  void SetStr(ConfKey key, const char* value, uindex length);
  void SetU8(ConfKey key, u8 value);
  void SetU16(ConfKey key, u16 value);
  void SetU32(ConfKey key, u32 value);
  void SetU64(ConfKey key, u64 value);
  void SetS8(ConfKey key, s8 value);
  void SetS16(ConfKey key, s16 value);
  void SetS32(ConfKey key, s32 value);
  void SetS64(ConfKey key, s64 value);
  void SetF32(ConfKey key, f32 value);
  void SetF64(ConfKey key, f64 value);
  void SetIndex(ConfKey key, uindex value);
  void SetUx(ConfKey key, ux value);
  void SetSx(ConfKey key, sx value);
  void SetFx(ConfKey key, fx value);
  void SetBool(ConfKey key, bool value);

 private:
  static constexpr const char* kConfigFilePath_{"./comet_config.cfg"};
  std::unordered_map<ConfKey, ConfValue> values_{};

  template <typename KeyString, typename ValueString>
  void ParseKeyValuePair(KeyString&& raw_key, ValueString&& raw_value) {
    auto key{COMET_STRING_ID(
        utils::string::GetTrimmedCopy(std::forward<KeyString>(raw_key)))};
    auto str_value{
        utils::string::GetTrimmedCopy(std::forward<KeyString>(raw_value))};

    if (key == kApplicationName || key == kRenderingDriver ||
        key == kResourceRootPath) {
      SetStr(key, str_value);
    } else if (key == kApplicationMajorVersion ||
               key == kApplicationMinorVersion ||
               key == kApplicationPatchVersion ||
               key == kRenderingWindowWidth || key == kRenderingWindowHeight ||
               key == kRenderingOpenGlMajorVersion ||
               key == kRenderingOpenGlMinorVersion ||
               key == kRenderingVulkanVariantVersion ||
               key == kRenderingVulkanMajorVersion ||
               key == kRenderingVulkanMinorVersion ||
               key == kRenderingVulkanPatchVersion ||
               key == kRenderingVulkanMaxFramesInFlight) {
      SetU16(key, utils::string::ParseU16(str_value));
    } else if (key == kRenderingClearColorR || key == kRenderingClearColorG ||
               key == kRenderingClearColorB || key == kRenderingClearColorA) {
      SetF32(key, utils::string::ParseF32(str_value));
    } else if (key == kCoreMsPerUpdate) {
      SetF64(key, utils::string::ParseF64(str_value));
    } else if (key == kRenderingIsVsync ||
               key == kRenderingIsSamplerAnisotropy ||
               key == kRenderingIsSampleRateShading) {
      SetBool(key, utils::string::ParseBool(str_value));
    } else {
      COMET_LOG_CORE_WARNING("Invalid configuration key: \"",
                             COMET_STRING_ID_LABEL(key), "\". Ignoring.");
    }
  }
};
}  // namespace conf
}  // namespace comet

#define COMET_CONF(key) Engine::Get().GetConfigurationManager().Get(key)
#define COMET_CONF_STR(key) Engine::Get().GetConfigurationManager().GetStr(key)
#define COMET_CONF_U8(key) Engine::Get().GetConfigurationManager().GetU8(key)
#define COMET_CONF_U16(key) Engine::Get().GetConfigurationManager().GetU16(key)
#define COMET_CONF_U32(key) Engine::Get().GetConfigurationManager().GetU32(key)
#define COMET_CONF_U64(key) Engine::Get().GetConfigurationManager().GetU64(key)
#define COMET_CONF_S8(key) Engine::Get().GetConfigurationManager().GetS8(key)
#define COMET_CONF_S16(key) Engine::Get().GetConfigurationManager().GetS16(key)
#define COMET_CONF_S32(key) Engine::Get().GetConfigurationManager().GetS32(key)
#define COMET_CONF_S64(key) Engine::Get().GetConfigurationManager().GetS64(key)
#define COMET_CONF_F32(key) Engine::Get().GetConfigurationManager().GetF32(key)
#define COMET_CONF_F64(key) Engine::Get().GetConfigurationManager().GetF64(key)
#define COMET_CONF_UINDEX(key) \
  Engine::Get().GetConfigurationManager().GetIndex(key)
#define COMET_CONF_UX(key) Engine::Get().GetConfigurationManager().GetUx(key)
#define COMET_CONF_SX(key) Engine::Get().GetConfigurationManager().GetSx(key)
#define COMET_CONF_FX(key) Engine::Get().GetConfigurationManager().GetFx(key)
#define COMET_CONF_BOOL(key) \
  Engine::Get().GetConfigurationManager().GetBool(key)

#endif  // COMET_COMET_CORE_CONF_CONFIGURATION_MANAGER_H_