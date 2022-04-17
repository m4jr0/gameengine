// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DRIVER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DRIVER_H_

#include "comet/event/event.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/window/glfw_window.h"
#include "comet_precompile.h"

namespace comet {
namespace rendering {
namespace vk {
struct VulkanDriverDescr : DriverDescr {};

class VulkanDriver : public Driver {
 public:
  explicit VulkanDriver(const VulkanDriverDescr& descr);
  VulkanDriver(const VulkanDriver&) = delete;
  VulkanDriver(VulkanDriver&&) = delete;
  VulkanDriver& operator=(const VulkanDriver&) = delete;
  VulkanDriver& operator=(VulkanDriver&&) = delete;
  virtual ~VulkanDriver() = default;

  virtual void Initialize() override;
  virtual void Destroy() override;
  virtual void Start() override;
  virtual void Update(
      time::Interpolation interpolation,
      game_object::GameObjectManager& game_object_manager) override;

  void SetSize(unsigned int width, unsigned int height);
  void OnEvent(const event::Event&);

  virtual bool IsInitialized() const override;
  virtual Window& GetWindow() override;

 private:
  unsigned int major_version_;
  unsigned int minor_version_;
  bool is_initialized_ = false;
  GlfwWindow window_;
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DRIVER_H_
