// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "opengl_driver.h"

#include "comet/event/event_manager.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/window/window_event.h"

namespace comet {
namespace rendering {
namespace gl {
OpenGlDriver::OpenGlDriver(const OpenGlDriverDescr& descr) : Driver(descr) {
  OpenGlGlfwWindowDescr window_descr{};
  window_descr.width = window_width_;
  window_descr.height = window_height_;
  SetName(window_descr, app_name_, app_name_len_);
  window_descr.opengl_major_version = descr.opengl_major_version;
  window_descr.opengl_minor_version = descr.opengl_minor_version;
  window_descr.is_vsync = is_vsync_;
  window_descr.anti_aliasing_type = anti_aliasing_type_;
  window_ = std::make_unique<OpenGlGlfwWindow>(window_descr);

  if (is_triple_buffering_) {
    COMET_LOG_RENDERING_WARNING(
        "Triple buffering is not currently supported on the OpenGL driver. "
        "Disabling it.");
    is_triple_buffering_ = false;
  }
}

void OpenGlDriver::Initialize() {
  Driver::Initialize();
  COMET_LOG_RENDERING_DEBUG("Initializing OpenGL driver.");
  window_->Initialize();
  COMET_ASSERT(window_->IsInitialized(), "Window could not be initialized!");

  event::EventManager::Get().Register(
      COMET_EVENT_BIND_FUNCTION(OpenGlDriver::OnEvent),
      WindowResizeEvent::kStaticType_);

  [[maybe_unused]] const auto result{
      gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))};

  COMET_ASSERT(result, "Could not load GL Loader!");

#ifdef COMET_DEBUG_RENDERING
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(LogOpenGlMessage, nullptr);
#endif  // COMET_DEBUG_RENDERING

  glEnable(GL_DEPTH_TEST);

  if (anti_aliasing_type_ != AntiAliasingType::None) {
    glEnable(GL_MULTISAMPLE);

    if (is_sample_rate_shading_) {
      glEnable(GL_SAMPLE_SHADING);
      glMinSampleShading(.2f);
    }
  }

  glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

  if (is_sampler_anisotropy_) {
#ifdef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    GLfloat max_anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
    glSamplerParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        max_anisotropy);
#else
    COMET_LOG_RENDERING_ERROR(
        "Sampler anisotropy is enabled, but current driver does not seem to "
        "support it. Ignoring feature.");
#endif  // GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
  }

  InitializeHandlers();
}

void OpenGlDriver::Shutdown() {
  DestroyHandlers();
  window_->Destroy();
  is_resize_ = false;
  frame_count_ = 0;
  Driver::Shutdown();
}

void OpenGlDriver::Update(frame::FramePacket* packet) {
  glClearColor(clear_color_[0], clear_color_[1], clear_color_[2],
               clear_color_[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  packet->is_rendering_skipped =
      packet->is_rendering_skipped || window_->IsFlat();

  HandleResize();
  window_->Update();
  Draw(packet);
  window_->SwapBuffers();
  ++frame_count_;
}

DriverType OpenGlDriver::GetType() const noexcept { return DriverType::OpenGl; }

void OpenGlDriver::InitializeHandlers() {
  MeshHandlerDescr mesh_handler_descr{};
  mesh_handler_ = std::make_unique<MeshHandler>(mesh_handler_descr);

  ShaderModuleHandlerDescr shader_module_handler_descr{};
  shader_module_handler_ =
      std::make_unique<ShaderModuleHandler>(shader_module_handler_descr);

  TextureHandlerDescr texture_handler_descr{};
  texture_handler_ = std::make_unique<TextureHandler>(texture_handler_descr);

  MaterialHandlerDescr material_handler_descr{};
  material_handler_descr.texture_handler = texture_handler_.get();
  material_handler_ = std::make_unique<MaterialHandler>(material_handler_descr);

  ShaderHandlerDescr shader_handler_descr{};
  shader_handler_descr.shader_module_handler = shader_module_handler_.get();
  shader_handler_descr.material_handler = material_handler_.get();
  shader_handler_descr.mesh_handler = mesh_handler_.get();
  shader_handler_descr.texture_handler = texture_handler_.get();
  shader_handler_ = std::make_unique<ShaderHandler>(shader_handler_descr);

  RenderProxyHandlerDescr render_proxy_handler_descr{};
  render_proxy_handler_descr.material_handler = material_handler_.get();
  render_proxy_handler_descr.mesh_handler = mesh_handler_.get();
  render_proxy_handler_descr.shader_handler = shader_handler_.get();
  render_proxy_handler_ =
      std::make_unique<RenderProxyHandler>(render_proxy_handler_descr);

  ViewHandlerDescr view_handler_descr{};
  view_handler_descr.shader_handler = shader_handler_.get();
  view_handler_descr.render_proxy_handler = render_proxy_handler_.get();
  view_handler_descr.rendering_view_descrs = &rendering_view_descrs_;
  view_handler_descr.window = window_.get();
  view_handler_ = std::make_unique<ViewHandler>(view_handler_descr);

  texture_handler_->Initialize();
  shader_module_handler_->Initialize();
  shader_handler_->Initialize();
  material_handler_->Initialize();
  mesh_handler_->Initialize();
  render_proxy_handler_->Initialize();
  view_handler_->Initialize();
}

void OpenGlDriver::DestroyHandlers() {
  // Order is important to improve performance.
  shader_module_handler_->Shutdown();
  texture_handler_->Shutdown();
  material_handler_->Shutdown();
  mesh_handler_->Shutdown();
  render_proxy_handler_->Shutdown();
  shader_handler_->Shutdown();
  view_handler_->Shutdown();

  shader_module_handler_ = nullptr;
  shader_handler_ = nullptr;
  texture_handler_ = nullptr;
  material_handler_ = nullptr;
  mesh_handler_ = nullptr;
  render_proxy_handler_ = nullptr;
  view_handler_ = nullptr;
}

void OpenGlDriver::SetSize(WindowSize, WindowSize) {
  // Postpone resizing until the Update method, where it's safe to modify due to
  // OpenGL's main thread constraints.
  is_resize_ = true;
}

void OpenGlDriver::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == WindowResizeEvent::kStaticType_) {
    const auto& window_resize_event{
        static_cast<const WindowResizeEvent&>(event)};
    SetSize(window_resize_event.GetWidth(), window_resize_event.GetHeight());
  }
}

Window* OpenGlDriver::GetWindow() { return window_.get(); }

u32 OpenGlDriver::GetDrawCount() const {
  return render_proxy_handler_->GetVisibleCount();
}

void OpenGlDriver::Draw(frame::FramePacket* packet) {
  COMET_PROFILE("OpenGlDriver::Draw");
  mesh_handler_->Update(packet);
  render_proxy_handler_->Update(packet);
  view_handler_->Update(packet);
}

void OpenGlDriver::HandleResize() {
  if (!is_resize_) {
    return;
  }

  glViewport(0, 0, window_->GetWidth(), window_->GetHeight());
  is_resize_ = false;
}

#ifdef COMET_DEBUG_RENDERING
void GLAPIENTRY OpenGlDriver::LogOpenGlMessage(GLenum, GLenum type, GLuint,
                                               GLenum severity, GLsizei,
                                               const GLchar* message,
                                               const void*) {
  constexpr auto kTypeStrLen{12};
  schar type_str[kTypeStrLen]{'\0'};

  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      Copy(type_str, "Error", 5);
      break;

    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      Copy(type_str, "Deprecated", 10);
      break;

    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      Copy(type_str, "Undefined", 9);
      break;

    case GL_DEBUG_TYPE_PORTABILITY:
      Copy(type_str, "Portability", 11);
      break;

    case GL_DEBUG_TYPE_PERFORMANCE:
      Copy(type_str, "Performance", 11);
      break;

    case GL_DEBUG_TYPE_MARKER:
      Copy(type_str, "Marker", 6);
      break;

    case GL_DEBUG_TYPE_PUSH_GROUP:
      Copy(type_str, "Push Group", 10);
      break;

    case GL_DEBUG_TYPE_POP_GROUP:
      Copy(type_str, "Pop Group", 9);
      break;

    case GL_DEBUG_TYPE_OTHER:
      Copy(type_str, "Other", 5);
      break;

    default:
      Copy(type_str, "???", 3);
      break;
  }

  constexpr auto kSeverityStrLen{13};
  schar severity_str[kSeverityStrLen]{'\0'};

  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      Copy(severity_str, "High", 4);
      COMET_LOG_RENDERING_ERROR("[", severity_str, " | ", type_str, "] ",
                                message);
      break;

    case GL_DEBUG_SEVERITY_MEDIUM:
      Copy(severity_str, "Medium", 6);
      COMET_LOG_RENDERING_ERROR("[", severity_str, " | ", type_str, "] ",
                                message);
      break;

    case GL_DEBUG_SEVERITY_LOW:
      Copy(severity_str, "Low", 3);
      COMET_LOG_RENDERING_WARNING("[", severity_str, " | ", type_str, "] ",
                                  message);
      break;

    default:
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      Copy(severity_str, "Notification", 12);
      COMET_LOG_RENDERING_DEBUG("[", severity_str, " | ", type_str, "] ",
                                message);
      break;
  }
}
#endif  // COMET_DEBUG_RENDERING
}  // namespace gl
}  // namespace rendering
}  // namespace comet
