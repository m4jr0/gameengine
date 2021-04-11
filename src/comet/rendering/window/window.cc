// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "window.h"

namespace comet {
namespace rendering {
Window::Window(unsigned int width, unsigned int height, const std::string& name)
    : width_(width), height_(height), name_(name) {}

const std::string Window::GetName() const noexcept { return name_; }

const unsigned int Window::GetWidth() const noexcept { return width_; }

const unsigned int Window::GetHeight() const noexcept { return height_; }

void Window::SetName(std::string name) { name_ = name; }

void Window::SetWidth(unsigned int width) { SetSize(width, height_); }

void Window::SetHeight(unsigned int height) { SetSize(width_, height); }
}  // namespace rendering
}  // namespace comet
