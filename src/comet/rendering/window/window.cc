// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "window.h"

#include "comet/core/logger.h"
#include "comet/event/event_manager.h"
#include "comet/event/window_event.h"

namespace comet {
namespace rendering {
void SetName(WindowDescr& descr, const schar* name, usize name_len) {
  descr.name_len = name_len;

  if (descr.name_len >= kMaxWindowNameLen) {
    COMET_LOG_RENDERING_WARNING(
        "Window name provided is too long: ", descr.name_len,
        " >= ", kMaxWindowNameLen, ". It will be truncated.");
    descr.name_len = static_cast<usize>(kMaxWindowNameLen - 1);
  }

  Copy(descr.name, name, descr.name_len);
  descr.name[descr.name_len + 1] = '\0';
}

Window::Window(const WindowDescr& descr)
    : width_{descr.width}, height_{descr.height} {
  Copy(name_, descr.name, descr.name_len);
  name_len_ = descr.name_len;
  event::EventManager::Get().FireEvent<event::WindowInitializedEvent>(width_,
                                                                      height_);
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

Window ::~Window() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for window, but it is still initialized!");
}

void Window ::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize window, but it is already done!");
  is_initialized_ = true;
}

void Window ::Destroy() {
  COMET_ASSERT(is_initialized_,
               "Tried to destroy window, but it is not initialized!");
  is_initialized_ = false;
}

void Window::Update() {}

bool Window::IsInitialized() const noexcept { return is_initialized_; }

const schar* Window::GetName() const noexcept { return name_; }

WindowSize Window::GetWidth() const noexcept { return width_; }

WindowSize Window::GetHeight() const noexcept { return height_; }
}  // namespace rendering
}  // namespace comet
