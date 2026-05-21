// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/platform/window/window.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/logger/logging.h"
#include "comet/event/event_manager.h"
#include "comet/rendering/window/window_event.h"

namespace comet {
namespace platform {
void SetName(WindowDescr& descr, const schar* name, usize name_len) {
  descr.name_len = name_len;

  if (descr.name_len >= kMaxWindowNameLen) {
    COMET_LOG_WARNING(LoggerType::Rendering, "window::SetName",
                      "window name is too long and will be truncated",
                      "name_len", descr.name_len, "max_name_len",
                      kMaxWindowNameLen);
    descr.name_len = static_cast<usize>(kMaxWindowNameLen - 1);
  }

  Copy(descr.name, name, descr.name_len);
  descr.name[descr.name_len] = '\0';
}

Window::Window(const WindowDescr& descr)
    : width_{descr.width}, height_{descr.height} {
  Copy(name_, descr.name, descr.name_len);
  name_len_ = descr.name_len;
  event::EventManager::Get().FireEvent<WindowInitializedEvent>(width_, height_);
}

Window::Window(Window&& other) noexcept
    : is_initialized_{other.is_initialized_},
      width_{other.width_},
      height_{other.height_} {
  Copy(name_, other.name_, other.name_len_);
  name_len_ = other.name_len_;
  other.is_initialized_ = false;
  other.width_ = 0;
  other.height_ = 0;
  Clear(other.name_, static_cast<usize>(kMaxWindowNameLen - 1));
  other.name_len_ = 0;
}

Window& Window::operator=(Window&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  is_initialized_ = other.is_initialized_;
  width_ = other.width_;
  height_ = other.height_;
  Copy(name_, other.name_, other.name_len_);
  name_len_ = other.name_len_;
  other.is_initialized_ = false;
  other.width_ = 0;
  other.height_ = 0;
  Clear(other.name_, static_cast<usize>(kMaxWindowNameLen - 1));
  other.name_len_ = 0;
  return *this;
}

Window::~Window() {
  COMET_ASSERT(!is_initialized_, "Window::~Window",
               "window is still initialized");
}

void Window::Initialize() {
  COMET_ASSERT(!is_initialized_, "Window::Initialize",
               "window is already initialized");

  OnInitialize();
  is_initialized_ = true;
}

void Window::Destroy() {
  COMET_ASSERT(is_initialized_, "Window::Destroy", "window is not initialized");

  OnDestroy();
  is_initialized_ = false;
}

void Window::Update() { OnUpdate(); }

bool Window::IsInitialized() const noexcept { return is_initialized_; }

bool Window::IsFlat() const noexcept { return width_ == 0 || height_ == 0; }

const schar* Window::GetName() const noexcept { return name_; }

WindowSize Window::GetWidth() const noexcept { return width_; }

WindowSize Window::GetHeight() const noexcept { return height_; }

void Window::OnInitialize() {}

void Window::OnDestroy() {}

void Window::OnUpdate() {}
}  // namespace platform
}  // namespace comet
