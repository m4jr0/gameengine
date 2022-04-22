// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_driver.h"

#include "boost/format.hpp"

#include "comet/core/engine.h"
#include "comet/event/event_manager.h"
#include "comet/event/input_event.h"
#include "comet/event/runtime_event.h"
#include "comet/event/window_event.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace rendering {
namespace gl {
OpenGlDriver::OpenGlDriver(const OpenGlDriverDescr& descr)
    : major_version_(descr.major_version), minor_version_(descr.minor_version) {
  OpenGlGlfwWindowDescr window_descr{};
  window_descr.width = descr.width;
  window_descr.height = descr.height;
  window_descr.name = descr.name;
  window_descr.opengl_major_version = descr.major_version;
  window_descr.opengl_minor_version = descr.minor_version;
  window_ = OpenGlGlfwWindow(window_descr);
}

void OpenGlDriver::Initialize() {
  COMET_LOG_RENDERING_DEBUG("Initializing OpenGL driver.");
  window_.Initialize();

  if (!window_.IsInitialized()) {
    return;
  }

  event::EventManager& event_manager =
      core::Engine::GetEngine().GetEventManager();

  event_manager.Register(COMET_EVENT_BIND_FUNCTION(OpenGlDriver::OnEvent),
                         event::WindowResizeEvent::kStaticType_);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    COMET_LOG_RENDERING_ERROR("Failed to initialize glad");
    event_manager.FireEventNow<event::UnrecoverableErrorEvent>();
  }

  glEnable(GL_DEPTH_TEST);
  is_initialized_ = true;
}

void OpenGlDriver::Destroy() { window_.Destroy(); }

void OpenGlDriver::Update(time::Interpolation interpolation,
                          game_object::GameObjectManager& game_object_manager) {
  glClearColor(0.001f, 0.001f, 0.001f, 0.001f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  game_object_manager.Update();
  window_.SwapBuffers();
}

void OpenGlDriver::SetSize(unsigned int width, unsigned int height) {
  window_.SetSize(width, height);

  glViewport(0, 0, static_cast<GLsizei>(window_.GetWidth()),
             static_cast<GLsizei>(window_.GetHeight()));
}

void OpenGlDriver::OnEvent(const event::Event& event) {
  const auto& event_type = event.GetType();

  if (event_type == event::WindowResizeEvent::kStaticType_) {
    const auto& window_event =
        static_cast<const event::WindowResizeEvent&>(event);
    SetSize(window_event.GetWidth(), window_event.GetHeight());
  }
}

bool OpenGlDriver::IsInitialized() const { return is_initialized_; }

Window& OpenGlDriver::GetWindow() { return window_; }
}  // namespace gl
}  // namespace rendering
}  // namespace comet
