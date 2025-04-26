// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_INPUT_INPUT_MANAGER_H_
#define COMET_COMET_INPUT_INPUT_MANAGER_H_

#include <atomic>

#define GLFW_INCLUDE_NONE
#include <optional>

#include "GLFW/glfw3.h"

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/concurrency/thread/thread.h"
#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/core/type/bitset.h"
#include "comet/input/input.h"
#include "comet/math/vector.h"

namespace comet {
namespace input {
namespace internal {
static constexpr usize kKeyBaseOffset{32};
static constexpr usize kKeyCount{348 - kKeyBaseOffset};
static constexpr usize kMouseButtonCount{8};

struct CachedInputState {
  Bitset keys_pressed{};
  Bitset keys_down{};
  Bitset keys_up{};
  Bitset mouse_buttons_pressed{};
  Bitset mouse_buttons_down{};
  Bitset mouse_buttons_up{};
  math::Vec2 mouse_position{};
};

struct UpdatedInputState {
  fiber::FiberMutex mtx{};
  std::optional<math::Vec2> new_position{};
  MouseCursorMode cursor_mode{MouseCursorMode::Unknown};
};
}  // namespace internal

class InputManager : public Manager {
 public:
  static InputManager& Get();

  InputManager() = default;
  InputManager(const InputManager&) = delete;
  InputManager(InputManager&&) = delete;
  InputManager& operator=(const InputManager&) = delete;
  InputManager& operator=(InputManager&&) = delete;
  virtual ~InputManager() = default;

  void Initialize() override;
  virtual void Shutdown() override;
  virtual void Update();
  virtual bool IsKeyPressed(KeyCode) const;
  virtual bool IsKeyUp(KeyCode) const;
  virtual bool IsKeyDown(KeyCode) const;
  virtual bool IsMousePressed(MouseButton key_code) const;
  virtual bool IsMouseDown(MouseButton key_code) const;
  virtual bool IsMouseUp(MouseButton key_code) const;
  virtual math::Vec2 GetMousePosition() const;
  virtual void SetMousePosition(f32, f32);
  virtual void EnableUnconstrainedMouseCursor();
  virtual void DisableUnconstrainedMouseCursor();
  void AttachGlfwWindow(GLFWwindow* window_handle_);

#ifdef COMET_IMGUI
  static void EnableImGui();
#endif  // COMET_IMGUI

  bool IsAltPressed() const;
  bool IsShiftPressed() const;

 private:
  void ReadInputs();
  void ApplyUserUpdates();

  thread::ThreadId thread_id_{thread::kInvalidThreadId};
  internal::CachedInputState cached_input_state_{};
  internal::UpdatedInputState updated_input_state_{};
  memory::PlatformAllocator cached_input_state_allocator_{
      memory::kEngineMemoryTagInput};
  mutable std::atomic<GLFWwindow*> window_handle_{nullptr};
#ifdef COMET_IMGUI
  static inline bool is_imgui_{false};
#endif  // COMET_IMGUI
};

class NullInputManager : public InputManager {
 public:
  NullInputManager() = default;
  NullInputManager(const NullInputManager&) = delete;
  NullInputManager(NullInputManager&&) = delete;
  NullInputManager& operator=(const NullInputManager&) = delete;
  NullInputManager& operator=(NullInputManager&&) = delete;
  virtual ~NullInputManager() = default;

  virtual bool IsKeyPressed(KeyCode) const override { return false; };

  virtual bool IsKeyUp(KeyCode) const override { return false; };
  virtual bool IsKeyDown(KeyCode) const override { return false; };
  virtual bool IsMousePressed(MouseButton) const override { return false; };
  virtual bool IsMouseDown(MouseButton) const override { return false; };
  virtual bool IsMouseUp(MouseButton) const override { return false; };

  virtual math::Vec2 GetMousePosition() const override {
    return math::Vec2{0.0f, 0.0f};
  };

  virtual void SetMousePosition(f32, f32) override{};
  virtual void EnableUnconstrainedMouseCursor() override {};
  virtual void DisableUnconstrainedMouseCursor() override {};
};
}  // namespace input
}  // namespace comet

#endif  // COMET_COMET_INPUT_INPUT_MANAGER_H_
