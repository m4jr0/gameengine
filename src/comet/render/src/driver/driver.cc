// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/driver.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/string/c_string.h"
#include "comet/core/memory/memory_utils.h"

namespace comet {
namespace render {
Driver::Driver(const DriverDescr& descr)
    : is_vsync_{descr.is_vsync},
      is_triple_buffering_{descr.is_triple_buffering},
      anti_aliasing_type_{descr.anti_aliasing_type},
      is_sampler_anisotropy_{anti_aliasing_type_ != AntiAliasingType::None &&
                             descr.is_sampler_anisotropy},
      is_sample_rate_shading_{anti_aliasing_type_ != AntiAliasingType::None &&
                              descr.is_sample_rate_shading},
      app_major_version_{descr.app_major_version},
      app_minor_version_{descr.app_minor_version},
      app_patch_version_{descr.app_patch_version},
      window_width_{descr.window_width},
      window_height_{descr.window_height},
      app_name_len_{descr.app_name_len},
      shadow_settings_{descr.shadow_settings},
      render_proxy_record_store_{descr.render_proxy_record_store} {
  COMET_ASSERT(shadow_settings_ != nullptr, "Driver::Driver",
               "shadow settings null");
  COMET_ASSERT(render_proxy_record_store_ != nullptr, "Driver::Driver",
               "render proxy record store is null");
  rendering_view_descrs_ = Array<RenderingViewDescr>::FromData(
      &rendering_view_descrs_allocator_, descr.rendering_view_descrs.GetData(),
      descr.rendering_view_descrs.GetSize());

  Copy(app_name_, descr.app_name, app_name_len_);
  memory::CopyMemory(clear_color_, descr.clear_color,
                     sizeof(descr.clear_color));
}

Driver::~Driver() {
  COMET_ASSERT(!is_initialized_, "Driver::~Driver",
               "driver is still initialized");
}

void Driver::Initialize() {
  COMET_ASSERT(!is_initialized_, "Driver::Initialize",
               "driver is already initialized");
  OnInitialize();
  is_initialized_ = true;
}

void Driver::Shutdown() {
  COMET_ASSERT(is_initialized_, "Driver::Shutdown",
               "driver is not initialized");
  OnShutdown();
  is_initialized_ = false;
}

bool Driver::IsInitialized() const noexcept { return is_initialized_; }

void Driver::OnInitialize() {}

void Driver::OnShutdown() {}
}  // namespace render
}  // namespace comet