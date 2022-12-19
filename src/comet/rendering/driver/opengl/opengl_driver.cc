// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_driver.h"

#include "comet/core/engine.h"
#include "comet/entity/component/mesh_component.h"
#include "comet/entity/component/transform_component.h"
#include "comet/event/event_manager.h"
#include "comet/event/input_event.h"
#include "comet/event/runtime_event.h"
#include "comet/event/window_event.h"
#include "comet/rendering/driver/opengl/opengl_mesh.h"

namespace comet {
namespace rendering {
namespace gl {
OpenGlDriver::OpenGlDriver(const OpenGlDriverDescr& descr)
    : major_version_{descr.major_version}, minor_version_{descr.minor_version} {
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

  auto& event_manager{Engine::Get().GetEventManager()};

  event_manager.Register(COMET_EVENT_BIND_FUNCTION(OpenGlDriver::OnEvent),
                         event::WindowResizeEvent::kStaticType_);

  COMET_ASSERT(
      gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)),
      "Could not load GL Loader!");
  glEnable(GL_DEPTH_TEST);
  InitializeShader();
  is_initialized_ = true;
}

void OpenGlDriver::Destroy() { window_.Destroy(); }

void OpenGlDriver::Update(time::Interpolation interpolation,
                          entity::EntityManager& entity_manager) {
  glClearColor(0.5f, 0.5f, 0.5f, 1.000f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // TODO(m4jr0): Remove temporary code.
  // Proxies should be managed with proper memory management and occlusion
  // culling.
  const auto view{
      entity_manager
          .GetView<entity::MeshComponent, entity::TransformComponent>()};

  for (const auto entity_id : view) {
    if (IsMeshProxy(entity_id)) {
      continue;
    }

    auto* mesh_cmp{
        entity_manager.GetComponent<entity::MeshComponent>(entity_id)};
    GenerateMeshProxy(entity_id, mesh_cmp->mesh, mesh_cmp->textures,
                      mesh_cmp->texture_count);
  }

  DrawMeshProxies();
  window_.SwapBuffers();
}

void OpenGlDriver::SetSize(u16 width, u16 height) {
  glViewport(0, 0, window_.GetWidth(), window_.GetHeight());
}

void OpenGlDriver::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == event::WindowResizeEvent::kStaticType_) {
    const auto& window_event{
        static_cast<const event::WindowResizeEvent&>(event)};
    SetSize(window_event.GetWidth(), window_event.GetHeight());
  }
}

bool OpenGlDriver::IsInitialized() const { return is_initialized_; }

Window& OpenGlDriver::GetWindow() { return window_; }
}  // namespace gl
}  // namespace rendering
}  // namespace comet
