// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONF_CONFIGURATION_MANAGER_H_
#define COMET_COMET_CORE_CONF_CONFIGURATION_MANAGER_H_

#include "comet/core/conf/configuration_value.h"
#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/type/map.h"
#include "comet/core/type/tstring.h"

namespace comet {
namespace conf {
using ConfValues = Map<ConfKey, ConfValue>;

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
  const ConfValue& Get(ConfKey key) const;
  const schar* GetStr(ConfKey key) const;
  const tchar* GetTStr(ConfKey key) const;
  void GetTStr(ConfKey key, TString& str) const;
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
  usize GetIndex(ConfKey key) const;
  ux GetUx(ConfKey key) const;
  sx GetSx(ConfKey key) const;
  fx GetFx(ConfKey key) const;
  bool GetBool(ConfKey key) const;

  void Set(ConfKey key, const ConfValue& value);

  void SetStr(ConfKey key, const schar* value);
  void SetStr(ConfKey key, const schar* value, usize length);
  void SetTStr(ConfKey key, const tchar* value);
  void SetTStr(ConfKey key, const tchar* value, usize length);
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
  void SetIndex(ConfKey key, usize value);
  void SetUx(ConfKey key, ux value);
  void SetSx(ConfKey key, sx value);
  void SetFx(ConfKey key, fx value);
  void SetBool(ConfKey key, bool value);
  void ParseKeyValuePair(schar* raw_key, usize raw_key_len, schar* value,
                         usize value_len);

 private:
  static constexpr auto kConfigFileRelativePath_{
      COMET_CTSTRING_VIEW("./comet_config.cfg")};
  memory::PlatformAllocator allocator_{memory::kEngineMemoryTagConfig};
  ConfValues values_{};
};
}  // namespace conf
}  // namespace comet

#define COMET_CONF(key) comet::conf::ConfigurationManager::Get().Get(key)
#define COMET_CONF_STR(key) comet::conf::ConfigurationManager::Get().GetStr(key)
#define COMET_CONF_TSTR(key, ...) \
  comet::conf::ConfigurationManager::Get().GetTStr(key, ##__VA_ARGS__)
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