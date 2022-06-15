// Copyright 2022 m4jr0. All Rights Reserved.
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

DriverType GetDriverTypeFromTypeName(const std::string& name);

class Driver {
 public:
  Driver() = default;
  Driver(const Driver&) = delete;
  Driver(Driver&&) = delete;
  Driver& operator=(const Driver&) = delete;
  Driver& operator=(Driver&&) = delete;
  virtual ~Driver() = default;

  virtual void Initialize() = 0;
  virtual void Destroy() = 0;
  virtual void Update(time::Interpolation interpolation,
                      entity::EntityManager& entity_manager) = 0;

  virtual bool IsInitialized() const = 0;
  virtual Window& GetWindow() = 0;
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_DRIVER_H_
