// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_PLATFORM_WINDOW_WINDOW_H_
#define COMET_PLATFORM_WINDOW_WINDOW_H_

#include "comet/core/essentials.h"
#include "comet/platform/window/window_common.h"

namespace comet {
namespace platform {
struct WindowDescr {
  WindowSize width{1280};
  WindowSize height{720};
  schar name[kMaxWindowNameLen]{'\0'};
  usize name_len{0};
};

void SetName(WindowDescr& descr, const schar* name, usize name_len);

class Window {
 public:
  Window() = delete;
  explicit Window(const WindowDescr& descr);
  Window(const Window&) = default;
  Window(Window&&) noexcept;
  Window& operator=(const Window&) = default;
  Window& operator=(Window&&) noexcept;
  virtual ~Window();

  void Initialize();
  void Destroy();

  void Update();

  virtual void SetSize(WindowSize width, WindowSize height) = 0;

  bool IsInitialized() const noexcept;

  bool IsFlat() const noexcept;

  virtual const schar* GetName() const noexcept;
  WindowSize GetWidth() const noexcept;
  WindowSize GetHeight() const noexcept;

 protected:
  virtual void OnInitialize();
  virtual void OnDestroy();

  virtual void OnUpdate();

  bool is_initialized_{false};
  WindowSize width_{0};
  WindowSize height_{0};
  schar name_[kMaxWindowNameLen]{'\0'};
  usize name_len_{0};
};
}  // namespace platform
}  // namespace comet

#endif  // COMET_PLATFORM_WINDOW_WINDOW_H_