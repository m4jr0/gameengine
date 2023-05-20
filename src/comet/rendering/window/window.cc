// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "window.h"

#include "comet/event/event_manager.h"
#include "comet/event/window_event.h"

namespace comet {
namespace rendering {
Window::Window(const WindowDescr& descr)
    : name_{descr.name}, width_{descr.width}, height_{descr.height} {
  event::EventManager::Get().FireEvent<event::WindowInitializedEvent>(width_,
                                                                      height_);
}

Window::Window(Window&& other) noexcept
    : is_initialized_{other.is_initialized_},
      width_{other.width_},
      height_{other.height_},
      name_{std::move(other.name_)} {
  other.is_initialized_ = false;
  other.width_ = 0;
  other.height_ = 0;
  other.name_.clear();
}

Window& Window::operator=(Window&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  is_initialized_ = other.is_initialized_;
  width_ = other.width_;
  height_ = other.height_;
  name_ = std::move(other.name_);
  other.is_initialized_ = false;
  other.width_ = 0;
  other.height_ = 0;
  other.name_.clear();
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

const std::string& Window::GetName() const noexcept { return name_; }

const WindowSize Window::GetWidth() const noexcept { return width_; }

const WindowSize Window::GetHeight() const noexcept { return height_; }
}  // namespace rendering
}  // namespace comet
