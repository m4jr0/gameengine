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
  u8 major_version;
  u8 minor_version;
};

class OpenGlDriver : public Driver {
 public:
  explicit OpenGlDriver(const OpenGlDriverDescr& descr);
  OpenGlDriver(const OpenGlDriver&) = delete;
  OpenGlDriver(OpenGlDriver&&) = delete;
  OpenGlDriver& operator=(const OpenGlDriver&) = delete;
  OpenGlDriver& operator=(OpenGlDriver&&) = delete;
  ~OpenGlDriver() = default;

  void Initialize() override;
  void Destroy() override;
  void Update(time::Interpolation interpolation,
              entity::EntityManager& entity_manager) override;

  void SetSize(u16 width, u16 height);
  void OnEvent(const event::Event&);

  bool IsInitialized() const override;
  Window& GetWindow() override;

 private:
  u8 major_version_{0};
  u8 minor_version_{0};
  bool is_initialized_{false};
  OpenGlGlfwWindow window_;
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DRIVER_H_