// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_DRIVER_H_
#define COMET_COMET_RENDERING_DRIVER_DRIVER_H_

#include "comet_precompile.h"

#include "comet/entity/entity_manager.h"
#include "comet/rendering/rendering_common.h"
#include "comet/rendering/window/window.h"
#include "comet/time/time_manager.h"

namespace comet {
namespace rendering {
enum class DriverType : u8 { Unknown = 0, OpenGl, Vulkan, Direct3d12 };

struct DriverDescr {
  DriverType type{DriverType::Unknown};
  bool is_vsync{false};
  bool is_triple_buffering{false};
  bool is_sampler_anisotropy{false};
  bool is_sample_rate_shading{false};
  u8 app_major_version{0};
  u8 app_minor_version{0};
  u8 app_patch_version{0};
  AntiAliasingType anti_aliasing_type{AntiAliasingType::None};
  WindowSize window_width{0};
  WindowSize window_height{0};
  f32 clear_color[4]{0.0f, 0.0f, 0.0f, 1.0f};
  std::string app_name{};
  std::vector<RenderingViewDescr> rendering_view_descrs{};
};

DriverType GetDriverTypeFromStr(std::string_view str);
AntiAliasingType GetAntiAliasingTypeFromStr(std::string_view str);

class Driver {
 public:
  explicit Driver(const DriverDescr& descr);
  Driver(const Driver&) = delete;
  Driver(Driver&&) = delete;
  Driver& operator=(const Driver&) = delete;
  Driver& operator=(Driver&&) = delete;
  virtual ~Driver();

  virtual void Initialize();
  virtual void Shutdown();
  virtual void Update(time::Interpolation interpolation) = 0;

  bool IsInitialized() const noexcept;
  virtual Window* GetWindow() = 0;

 protected:
  bool is_initialized_{false};
  bool is_vsync_{false};
  bool is_triple_buffering_{false};
  bool is_sampler_anisotropy_{false};
  bool is_sample_rate_shading_{false};
  u8 app_major_version_{0};
  u8 app_minor_version_{0};
  u8 app_patch_version_{0};
  AntiAliasingType anti_aliasing_type_{AntiAliasingType::None};
  WindowSize window_width_{0};
  WindowSize window_height_{0};
  f32 clear_color_[4]{0.0f, 0.0f, 0.0f, 1.0f};
  std::string app_name_{};
  std::vector<RenderingViewDescr> rendering_view_descrs_{};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_DRIVER_H_
