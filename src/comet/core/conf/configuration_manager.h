// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONF_CONFIGURATION_MANAGER_H_
#define COMET_COMET_CORE_CONF_CONFIGURATION_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/conf/configuration_value.h"
#include "comet/core/manager.h"
#include "comet/core/string.h"

using namespace std::literals;

namespace comet {
namespace conf {
class ConfigurationManager : public Manager {
 public:
  static ConfigurationManager& Get();

  ConfigurationManager() = default;
  ConfigurationManager(const ConfigurationManager&) = delete;
  ConfigurationManager(ConfigurationManager&&) = delete;
  ConfigurationManager& operator=(const ConfigurationManager&) = delete;
  ConfigurationManager& operator=(ConfigurationManager&&) = delete;
  virtual ~ConfigurationManager() = default;

  void Initialize() override;
  void Shutdown() override;
  void ParseConfFile();

  ConfValue& Get(ConfKey key);
  ConfValue Get(ConfKey key) const;
  std::string GetStr(ConfKey key) const;
  u8 GetU8(ConfKey key) const;
  u16 GetU16(ConfKey key) const;
  u32 GetU32(ConfKey key) const;
  u64 GetU64(ConfKey key) const;
  s8 GetS8(ConfKey key) const;
  s16 GetS16(ConfKey key) const;
  s32 GetS32(ConfKey key) const;
  s64 GetS64(ConfKey key) const;
  f32 GetF32(ConfKey key) const;
  f64 GetF64(ConfKey key) const;
  uindex GetIndex(ConfKey key) const;
  ux GetUx(ConfKey key) const;
  sx GetSx(ConfKey key) const;
  fx GetFx(ConfKey key) const;
  bool GetBool(ConfKey key) const;

  void Set(ConfKey key, const ConfValue& value);

  void SetStr(ConfKey key, const std::string& value);
  void SetStr(ConfKey key, const schar* value);
  void SetStr(ConfKey key, const schar* value, uindex length);
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
  template <typename KeyString, typename ValueString>
  void ParseKeyValuePair(KeyString&& raw_key, ValueString&& raw_value) {
    auto key{COMET_STRING_ID(GetTrimmedCopy(std::forward<KeyString>(raw_key)))};
    auto str_value{GetTrimmedCopy(std::forward<KeyString>(raw_value))};

    if (key == kApplicationName || key == kRenderingDriver ||
        key == kRenderingAntiAliasing || key == kResourceRootPath) {
      SetStr(key, str_value);
    } else if (key == kEventMaxQueueSize || key == kApplicationMajorVersion ||
               key == kApplicationMinorVersion ||
               key == kApplicationPatchVersion ||
               key == kRenderingWindowWidth || key == kRenderingWindowHeight ||
               key == kRenderingFpsCap || key == kRenderingOpenGlMajorVersion ||
               key == kRenderingOpenGlMinorVersion ||
               key == kRenderingVulkanVariantVersion ||
               key == kRenderingVulkanMajorVersion ||
               key == kRenderingVulkanMinorVersion ||
               key == kRenderingVulkanPatchVersion ||
               key == kRenderingVulkanMaxFramesInFlight) {
      SetU16(key, ParseU16(str_value));
    } else if (key == kRenderingClearColorR || key == kRenderingClearColorG ||
               key == kRenderingClearColorB || key == kRenderingClearColorA) {
      SetF32(key, ParseF32(str_value));
    } else if (key == kCoreMsPerUpdate) {
      SetF64(key, ParseF64(str_value));
    } else if (key == kRenderingIsVsync || key == kRenderingIsTripleBuffering ||
               key == kRenderingIsSamplerAnisotropy ||
               key == kRenderingIsSampleRateShading) {
      SetBool(key, ParseBool(str_value));
    } else {
      COMET_LOG_CORE_WARNING("Invalid configuration key: \"",
                             COMET_STRING_ID_LABEL(key), "\". Ignoring.");
    }
  }

  static constexpr auto kConfigFilePath_{"./comet_config.cfg"sv};
  std::unordered_map<ConfKey, ConfValue> values_{};
};
}  // namespace conf
}  // namespace comet

#define COMET_CONF(key) comet::conf::ConfigurationManager::Get().Get(key)
#define COMET_CONF_STR(key) comet::conf::ConfigurationManager::Get().GetStr(key)
#define COMET_CONF_U8(key) comet::conf::ConfigurationManager::Get().GetU8(key)
#define COMET_CONF_U16(key) comet::conf::ConfigurationManager::Get().GetU16(key)
#define COMET_CONF_U32(key) comet::conf::ConfigurationManager::Get().GetU32(key)
#define COMET_CONF_U64(key) comet::conf::ConfigurationManager::Get().GetU64(key)
#define COMET_CONF_S8(key) comet::conf::ConfigurationManager::Get().GetS8(key)
#define COMET_CONF_S16(key) comet::conf::ConfigurationManager::Get().GetS16(key)
#define COMET_CONF_S32(key) comet::conf::ConfigurationManager::Get().GetS32(key)
#define COMET_CONF_S64(key) comet::conf::ConfigurationManager::Get().GetS64(key)
#define COMET_CONF_F32(key) comet::conf::ConfigurationManager::Get().GetF32(key)
#define COMET_CONF_F64(key) comet::conf::ConfigurationManager::Get().GetF64(key)
#define COMET_CONF_UINDEX(key) \
  comet::conf::ConfigurationManager::Get().GetIndex(key)
#define COMET_CONF_UX(key) comet::conf::ConfigurationManager::Get().GetUx(key)
#define COMET_CONF_SX(key) comet::conf::ConfigurationManager::Get().GetSx(key)
#define COMET_CONF_FX(key) comet::conf::ConfigurationManager::Get().GetFx(key)
#define COMET_CONF_BOOL(key) \
  comet::conf::ConfigurationManager::Get().GetBool(key)

#endif  // COMET_COMET_CORE_CONF_CONFIGURATION_MANAGER_H_