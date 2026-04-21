// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_driver.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"
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
    COMET_LOG_WARNING(LoggerType::Rendering, "OpenGlDriver::OpenGlDriver",
                      "triple buffering not supported", "driver", "OpenGL");
    is_triple_buffering_ = false;
  }
}

void OpenGlDriver::Update(frame::FramePacket* packet) {
  COMET_ASSERT(packet != nullptr, "OpenGlDriver::Update",
               "frame packet is null");
  COMET_ASSERT(window_ != nullptr, "OpenGlDriver::Update", "window is null");
  COMET_ASSERT(frame_state_ != nullptr, "OpenGlDriver::Update",
               "frame state is null");

  window_->Update();
  HandlePresentationState(packet);
  PreDraw(packet);
  Draw(packet);
  PostDraw(packet);
  frame_state_->GoToNextFrame();
}

DriverType OpenGlDriver::GetType() const noexcept { return DriverType::OpenGl; }

void OpenGlDriver::SetSize(WindowSize, WindowSize) {
  // OpenGL mutations stay deferred to the main update path.
  is_resize_ = true;
}

void OpenGlDriver::OnEvent(const event::Event& event) {
  if (event.GetType() != WindowResizeEvent::kStaticType_) {
    return;
  }

  const auto& resize_event{static_cast<const WindowResizeEvent&>(event)};
  SetSize(resize_event.GetWidth(), resize_event.GetHeight());
}

void OpenGlDriver::RegisterEvents() {
  window_resize_listener_id_ = event::EventManager::Get().Register(
      [this](const event::Event& event) { OnEvent(event); },
      WindowResizeEvent::kStaticType_);
  COMET_ASSERT(window_resize_listener_id_ != event::kInvalidEventListenerId,
               "OpenGlDriver::RegisterEvents",
               "window resize listener registration failed");
}

void OpenGlDriver::UnregisterEvents() {
  if (window_resize_listener_id_ != event::kInvalidEventListenerId) {
    event::EventManager::Get().Unregister(window_resize_listener_id_);
    window_resize_listener_id_ = event::kInvalidEventListenerId;
  }
}

Window* OpenGlDriver::GetWindow() { return window_.get(); }

u32 OpenGlDriver::GetDrawCount() const {
  return render_proxy_handler_ != nullptr
             ? render_proxy_handler_->GetVisibleCount()
             : 0;
}

void OpenGlDriver::OnInitialize() {
  COMET_LOG_DEBUG(LoggerType::Rendering, "OpenGlDriver::OnInitialize",
                  "initializing opengl driver");

  COMET_ASSERT(window_ != nullptr, "OpenGlDriver::OnInitialize",
               "window is null");

  window_->Initialize();

  COMET_ASSERT(window_->IsInitialized(), "OpenGlDriver::OnInitialize",
               "window not initialized");

  RegisterEvents();

  [[maybe_unused]] const auto result{
      gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))};

  COMET_ASSERT(result != 0, "OpenGlDriver::OnInitialize",
               "failed to load gl loader");

#ifdef COMET_DEBUG_RENDERING
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(LogOpenGlMessage, nullptr);
#endif  // COMET_DEBUG_RENDERING

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  if (anti_aliasing_type_ != AntiAliasingType::None) {
    glEnable(GL_MULTISAMPLE);

#ifdef GL_SAMPLE_SHADING
    if (is_sample_rate_shading_) {
      glEnable(GL_SAMPLE_SHADING);
      glMinSampleShading(.2f);
    }
#endif  // GL_SAMPLE_SHADING
  }

#ifdef COMET_RENDERING_OPENGL_CLIP_CONTROL_ZERO_TO_ONE
  glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
#endif  // COMET_RENDERING_OPENGL_CLIP_CONTROL_ZERO_TO_ONE

  glEnable(GL_FRAMEBUFFER_SRGB);

  FrameStateDescr frame_state_descr{};
  frame_state_descr.max_frames_in_flight = static_cast<FrameInFlightIndex>(2);
  frame_state_ = std::make_unique<FrameState>(frame_state_descr);
  frame_state_->Initialize();

  InitializeHandlers();
  ApplyWindowResize();
}

void OpenGlDriver::OnShutdown() {
  UnregisterEvents();
  DestroyHandlers();

  if (window_ != nullptr && window_->IsInitialized()) {
    window_->Destroy();
  }

  frame_state_->Destroy();
  frame_state_ = nullptr;
  is_resize_ = false;
}

void OpenGlDriver::InitializeHandlers() {
  SamplerHandlerDescr sampler_handler_descr{};
  sampler_handler_descr.frame_state = frame_state_.get();
  sampler_handler_ = std::make_unique<SamplerHandler>(sampler_handler_descr);

  TextureHandlerDescr texture_handler_descr{};
  texture_handler_descr.frame_state = frame_state_.get();
  texture_handler_ = std::make_unique<TextureHandler>(texture_handler_descr);

  ShaderModuleHandlerDescr shader_module_handler_descr{};
  shader_module_handler_descr.frame_state = frame_state_.get();
  shader_module_handler_ =
      std::make_unique<ShaderModuleHandler>(shader_module_handler_descr);

  MaterialHandlerDescr material_handler_descr{};
  material_handler_descr.frame_state = frame_state_.get();
  material_handler_descr.texture_handler = texture_handler_.get();
  material_handler_descr.sampler_handler = sampler_handler_.get();
  material_handler_ = std::make_unique<MaterialHandler>(material_handler_descr);

  MeshHandlerDescr mesh_handler_descr{};
  mesh_handler_descr.frame_state = frame_state_.get();
  mesh_handler_ = std::make_unique<MeshHandler>(mesh_handler_descr);

  ShaderHandlerDescr shader_handler_descr{};
  shader_handler_descr.frame_state = frame_state_.get();
  shader_handler_descr.shader_module_handler = shader_module_handler_.get();
  shader_handler_descr.material_handler = material_handler_.get();
  shader_handler_descr.texture_handler = texture_handler_.get();
  shader_handler_descr.sampler_handler = sampler_handler_.get();
  shader_handler_ = std::make_unique<ShaderHandler>(shader_handler_descr);

  LightingHandlerDescr lighting_handler_descr{};
  lighting_handler_descr.frame_state = frame_state_.get();
  lighting_handler_descr.shadow_settings = shadow_settings_;
  lighting_handler_descr.texture_handler = texture_handler_.get();
  lighting_handler_descr.sampler_handler = sampler_handler_.get();
  lighting_handler_ = std::make_unique<LightingHandler>(lighting_handler_descr);

  RenderProxyHandlerDescr render_proxy_handler_descr{};
  render_proxy_handler_descr.frame_state = frame_state_.get();
  render_proxy_handler_descr.material_handler = material_handler_.get();
  render_proxy_handler_descr.mesh_handler = mesh_handler_.get();
  render_proxy_handler_descr.shader_handler = shader_handler_.get();
  render_proxy_handler_ =
      std::make_unique<RenderProxyHandler>(render_proxy_handler_descr);

  ViewHandlerDescr view_handler_descr{};
  view_handler_descr.frame_state = frame_state_.get();
  view_handler_descr.shadow_settings = shadow_settings_;
  view_handler_descr.shader_handler = shader_handler_.get();
  view_handler_descr.material_handler = material_handler_.get();
  view_handler_descr.render_proxy_handler = render_proxy_handler_.get();
  view_handler_descr.mesh_handler = mesh_handler_.get();
  view_handler_descr.lighting_handler = lighting_handler_.get();
  view_handler_descr.window = window_.get();
  view_handler_descr.rendering_view_descrs = &rendering_view_descrs_;
  view_handler_ = std::make_unique<ViewHandler>(view_handler_descr);

  texture_handler_->Initialize();
  shader_module_handler_->Initialize();
  material_handler_->Initialize();
  mesh_handler_->Initialize();
  sampler_handler_->Initialize();
  shader_handler_->Initialize();
  lighting_handler_->Initialize();
  render_proxy_handler_->Initialize();
  view_handler_->Initialize();
}

void OpenGlDriver::DestroyHandlers() {
  if (view_handler_ != nullptr) {
    view_handler_->Shutdown();
    view_handler_ = nullptr;
  }

  if (lighting_handler_ != nullptr) {
    lighting_handler_->Shutdown();
    lighting_handler_ = nullptr;
  }

  if (render_proxy_handler_ != nullptr) {
    render_proxy_handler_->Shutdown();
    render_proxy_handler_ = nullptr;
  }

  if (material_handler_ != nullptr) {
    material_handler_->Shutdown();
    material_handler_ = nullptr;
  }

  if (mesh_handler_ != nullptr) {
    mesh_handler_->Shutdown();
    mesh_handler_ = nullptr;
  }

  if (sampler_handler_ != nullptr) {
    sampler_handler_->Shutdown();
    sampler_handler_ = nullptr;
  }

  if (shader_handler_ != nullptr) {
    shader_handler_->Shutdown();
    shader_handler_ = nullptr;
  }

  if (texture_handler_ != nullptr) {
    texture_handler_->Shutdown();
    texture_handler_ = nullptr;
  }

  if (shader_module_handler_ != nullptr) {
    shader_module_handler_->Shutdown();
    shader_module_handler_ = nullptr;
  }
}

void OpenGlDriver::ApplyWindowResize() {
  COMET_ASSERT(window_ != nullptr, "OpenGlDriver::ApplyWindowResize",
               "window is null");

  if (!is_resize_) {
    return;
  }

  const auto width{window_->GetWidth()};
  const auto height{window_->GetHeight()};

  glViewport(0, 0, width, height);

  if (view_handler_ != nullptr && view_handler_->IsInitialized()) {
    view_handler_->SetSize(width, height);
  }

  is_resize_ = false;
}

void OpenGlDriver::PreDraw(frame::FramePacket* packet) {
  COMET_PROFILE("OpenGlDriver::PreDraw");
  COMET_ASSERT(packet != nullptr, "OpenGlDriver::PreDraw",
               "frame packet is null");

  ApplyWindowResize();

  glClearColor(clear_color_[0], clear_color_[1], clear_color_[2],
               clear_color_[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (!packet->can_present) {
    return;
  }
}

void OpenGlDriver::PostDraw(const frame::FramePacket* packet) {
  COMET_PROFILE("OpenGlDriver::PostDraw");
  COMET_ASSERT(packet != nullptr, "OpenGlDriver::PostDraw",
               "frame packet is null");

  if (!packet->can_present) {
    return;
  }

  window_->SwapBuffers();
}

void OpenGlDriver::Draw(frame::FramePacket* packet) {
  COMET_PROFILE("OpenGlDriver::Draw");
  COMET_ASSERT(packet != nullptr, "OpenGlDriver::Draw", "frame packet is null");

  UpdateGpuSceneState(packet);

  if (packet->can_present) {
    RecordFrame(packet);
  }
}

void OpenGlDriver::HandlePresentationState(frame::FramePacket* packet) {
  COMET_ASSERT(packet != nullptr, "OpenGlDriver::HandlePresentationState",
               "frame packet is null");

  if (window_->IsFlat()) {
    packet->can_present = false;
  }
}

void OpenGlDriver::UpdateGpuSceneState(frame::FramePacket* packet) {
  COMET_PROFILE("OpenGlDriver::UpdateGpuSceneState");
  COMET_ASSERT(packet != nullptr, "OpenGlDriver::UpdateGpuSceneState",
               "frame packet is null");

  mesh_handler_->Update(packet);
  lighting_handler_->Update(packet);
  render_proxy_handler_->Update(packet);
}

void OpenGlDriver::RecordFrame(frame::FramePacket* packet) {
  COMET_PROFILE("OpenGlDriver::RecordFrame");
  COMET_ASSERT(packet != nullptr, "OpenGlDriver::RecordFrame",
               "frame packet is null");
  view_handler_->Update(packet);
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
      Copy(type_str, "error", 5);
      break;

    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      Copy(type_str, "deprecated", 10);
      break;

    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      Copy(type_str, "undefined", 9);
      break;

    case GL_DEBUG_TYPE_PORTABILITY:
      Copy(type_str, "portability", 11);
      break;

    case GL_DEBUG_TYPE_PERFORMANCE:
      Copy(type_str, "performance", 11);
      break;

    case GL_DEBUG_TYPE_MARKER:
      Copy(type_str, "marker", 6);
      break;

    case GL_DEBUG_TYPE_PUSH_GROUP:
      Copy(type_str, "push_group", 10);
      break;

    case GL_DEBUG_TYPE_POP_GROUP:
      Copy(type_str, "pop_group", 9);
      break;

    case GL_DEBUG_TYPE_OTHER:
      Copy(type_str, "other", 5);
      break;

    default:
      Copy(type_str, kUnknownLabel, kUnknownLabelLen);
      break;
  }

  constexpr auto kSeverityStrLen{13};
  schar severity_str[kSeverityStrLen]{'\0'};

  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      Copy(severity_str, "High", 4);
      COMET_LOG_ERROR(LoggerType::Rendering, "OpenGlDriver::LogOpenGlMessage",
                      "opengl debug message", "severity", severity_str, "type",
                      type_str, "message", message);
      break;

    case GL_DEBUG_SEVERITY_MEDIUM:
      Copy(severity_str, "Medium", 6);
      COMET_LOG_ERROR(LoggerType::Rendering, "OpenGlDriver::LogOpenGlMessage",
                      "opengl debug message", "severity", severity_str, "type",
                      type_str, "message", message);
      break;

    case GL_DEBUG_SEVERITY_LOW:
      Copy(severity_str, "Low", 3);
      COMET_LOG_WARNING(LoggerType::Rendering, "OpenGlDriver::LogOpenGlMessage",
                        "opengl debug message", "severity", severity_str,
                        "type", type_str, "message", message);
      break;

    default:
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      Copy(severity_str, "Notification", 12);
      COMET_LOG_DEBUG(LoggerType::Rendering, "OpenGlDriver::LogOpenGlMessage",
                      "opengl debug message", "severity", severity_str, "type",
                      type_str, "message", message);
      break;
  }
}
#endif  // COMET_DEBUG_RENDERING
}  // namespace gl
}  // namespace rendering
}  // namespace comet