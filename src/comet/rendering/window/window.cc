// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "window.h"

namespace comet {
namespace rendering {
Window::Window(const WindowDescr& descr)
    : name_{descr.name}, width_{descr.width}, height_{descr.height} {}

const std::string Window::GetName() const noexcept { return name_; }

const WindowSize Window::GetWidth() const noexcept { return width_; }

const WindowSize Window::GetHeight() const noexcept { return height_; }
}  // namespace rendering
}  // namespace comet
