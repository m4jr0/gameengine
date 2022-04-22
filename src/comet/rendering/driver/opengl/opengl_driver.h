// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DRIVER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DRIVER_H_

#include "comet_precompile.h"

#include "glad/glad.h"

#include "comet/event/event.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/window/glfw/opengl/opengl_glfw_window.h"

namespace comet {
namespace rendering {
namespace gl {
struct OpenGlDriverDescr : DriverDescr {
  unsigned int major_version;
  unsigned int minor_version;
};

class OpenGlDriver : public Driver {
 public:
  explicit OpenGlDriver(const OpenGlDriverDescr& descr);
  OpenGlDriver(const OpenGlDriver&) = delete;
  OpenGlDriver(OpenGlDriver&&) = delete;
  OpenGlDriver& operator=(const OpenGlDriver&) = delete;
  OpenGlDriver& operator=(OpenGlDriver&&) = delete;
  virtual ~OpenGlDriver() = default;

  virtual void Initialize() override;
  virtual void Destroy() override;
  virtual void Update(
      time::Interpolation interpolation,
      game_object::GameObjectManager& game_object_manager) override;

  void SetSize(unsigned int width, unsigned int height);
  void OnEvent(const event::Event&);

  virtual bool IsInitialized() const override;
  virtual Window& GetWindow() override;

 private:
  unsigned int major_version_ = 0;
  unsigned int minor_version_ = 0;
  bool is_initialized_ = false;
  OpenGlGlfwWindow window_;
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DRIVER_H_
