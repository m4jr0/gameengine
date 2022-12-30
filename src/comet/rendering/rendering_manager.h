// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDERING_MANAGER_H_
#define COMET_COMET_RENDERING_RENDERING_MANAGER_H_

#include "comet_precompile.h"

#include "comet/entity/entity_manager.h"
#include "comet/rendering/driver/driver.h"
#include "comet/time/time_manager.h"

namespace comet {
namespace rendering {
class RenderingManager {
 public:
  RenderingManager() = default;
  RenderingManager(const RenderingManager&) = delete;
  RenderingManager(RenderingManager&&) = delete;
  RenderingManager& operator=(const RenderingManager&) = delete;
  RenderingManager& operator=(RenderingManager&&) = delete;
  ~RenderingManager() = default;

  void Initialize();
  void Destroy();
  void Update(time::Interpolation interpolation,
              entity::EntityManager& entity_manager);

  const Window* GetWindow() const;

 private:
  u32 counter_{0};
  f64 current_time_{0};
  std::unique_ptr<Driver> driver_{nullptr};

  void GenerateOpenGlDriver();
  void GenerateVulkanDriver();
  void GenerateDirect3D12Driver();
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_RENDERING_MANAGER_H_
