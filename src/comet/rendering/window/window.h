// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_WINDOW_WINDOW_H_
#define COMET_COMET_RENDERING_WINDOW_WINDOW_H_

#include "comet_precompile.h"

#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
struct WindowDescr {
  WindowSize width{1280};
  WindowSize height{720};
  schar name[kMaxWindowNameLen]{'\0'};
  uindex name_len{0};
};

void SetName(WindowDescr& descr, const schar* name, uindex name_len);

class Window {
 public:
  Window() = delete;
  explicit Window(const WindowDescr& descr);
  Window(const Window&) = default;
  Window(Window&&) noexcept;
  Window& operator=(const Window&) = default;
  Window& operator=(Window&&) noexcept;
  virtual ~Window();

  virtual void Initialize();
  virtual void Destroy();
  virtual void Update();
  virtual void SetSize(WindowSize width, WindowSize height) = 0;

  bool IsInitialized() const noexcept;
  virtual const schar* GetName() const noexcept;
  virtual WindowSize GetWidth() const noexcept;
  virtual WindowSize GetHeight() const noexcept;

 protected:
  bool is_initialized_{false};
  WindowSize width_{0};
  WindowSize height_{0};
  schar name_[kMaxWindowNameLen]{'\0'};
  uindex name_len_{0};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_WINDOW_WINDOW_H_