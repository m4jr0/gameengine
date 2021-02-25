// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_INPUT_INPUT_MANAGER_HPP_
#define KOMA_CORE_INPUT_INPUT_MANAGER_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <core/manager.hpp>

namespace koma {
// Here, we wrap the GLFW "input manager" to make it easier if we want to
// change it, some day.
enum class KeyCode {
  UNKNOWN = GLFW_KEY_UNKNOWN,
  SPACE = GLFW_KEY_SPACE,
  APOSTROPHE = GLFW_KEY_APOSTROPHE,
  COMMA = GLFW_KEY_COMMA,
  MINUS = GLFW_KEY_MINUS,
  PERIOD = GLFW_KEY_PERIOD,
  SLASH = GLFW_KEY_SLASH,
  ZERO = GLFW_KEY_0,
  ONE = GLFW_KEY_1,
  TWO = GLFW_KEY_2,
  THREE = GLFW_KEY_3,
  FOUR = GLFW_KEY_4,
  FIVE = GLFW_KEY_5,
  SIX = GLFW_KEY_6,
  SEVEN = GLFW_KEY_7,
  EIGHT = GLFW_KEY_8,
  NINE = GLFW_KEY_9,
  SEMICOLON = GLFW_KEY_SEMICOLON,
  EQUAL = GLFW_KEY_EQUAL,
  A = GLFW_KEY_A,
  B = GLFW_KEY_B,
  C = GLFW_KEY_C,
  D = GLFW_KEY_D,
  E = GLFW_KEY_E,
  F = GLFW_KEY_F,
  G = GLFW_KEY_G,
  H = GLFW_KEY_H,
  I = GLFW_KEY_I,
  J = GLFW_KEY_J,
  K = GLFW_KEY_K,
  L = GLFW_KEY_L,
  M = GLFW_KEY_M,
  N = GLFW_KEY_N,
  O = GLFW_KEY_O,
  P = GLFW_KEY_P,
  Q = GLFW_KEY_Q,
  R = GLFW_KEY_R,
  S = GLFW_KEY_S,
  T = GLFW_KEY_T,
  U = GLFW_KEY_U,
  V = GLFW_KEY_V,
  W = GLFW_KEY_W,
  X = GLFW_KEY_X,
  Y = GLFW_KEY_Y,
  Z = GLFW_KEY_Z,
  LEFT_BRACKET = GLFW_KEY_LEFT_BRACKET,
  BACKSLASH = GLFW_KEY_BACKSLASH,
  RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET,
  GRAVE_ACCENT = GLFW_KEY_GRAVE_ACCENT,
  WORLD_1 = GLFW_KEY_WORLD_1,
  WORLD_2 = GLFW_KEY_WORLD_2,
  ESCAPE = GLFW_KEY_ESCAPE,
  ENTER = GLFW_KEY_ENTER,
  TAB = GLFW_KEY_TAB,
  BACKSPACE = GLFW_KEY_BACKSPACE,
  INSERT = GLFW_KEY_INSERT,
  DEL = GLFW_KEY_DELETE,
  RIGHT = GLFW_KEY_RIGHT,
  LEFT = GLFW_KEY_LEFT,
  DOWN = GLFW_KEY_DOWN,
  UP = GLFW_KEY_UP,
  PAGE_UP = GLFW_KEY_PAGE_UP,
  PAGE_DOWN = GLFW_KEY_PAGE_DOWN,
  HOME = GLFW_KEY_HOME,
  END = GLFW_KEY_END,
  CAPS_LOCK = GLFW_KEY_CAPS_LOCK,
  SCROLL_LOCK = GLFW_KEY_SCROLL_LOCK,
  NUM_LOCK = GLFW_KEY_NUM_LOCK,
  PRINT_SCREEN = GLFW_KEY_PRINT_SCREEN,
  PAUSE = GLFW_KEY_PAUSE,
  F1 = GLFW_KEY_F1,
  F2 = GLFW_KEY_F2,
  F3 = GLFW_KEY_F3,
  F4 = GLFW_KEY_F4,
  F5 = GLFW_KEY_F5,
  F6 = GLFW_KEY_F6,
  F7 = GLFW_KEY_F7,
  F8 = GLFW_KEY_F8,
  F9 = GLFW_KEY_F9,
  F10 = GLFW_KEY_F10,
  F11 = GLFW_KEY_F11,
  F12 = GLFW_KEY_F12,
  F13 = GLFW_KEY_F13,
  F14 = GLFW_KEY_F14,
  F15 = GLFW_KEY_F15,
  F16 = GLFW_KEY_F16,
  F17 = GLFW_KEY_F17,
  F18 = GLFW_KEY_F18,
  F19 = GLFW_KEY_F19,
  F20 = GLFW_KEY_F20,
  F21 = GLFW_KEY_F21,
  F22 = GLFW_KEY_F22,
  F23 = GLFW_KEY_F23,
  F24 = GLFW_KEY_F24,
  F25 = GLFW_KEY_F25,
  KP_0 = GLFW_KEY_KP_0,
  KP_1 = GLFW_KEY_KP_1,
  KP_2 = GLFW_KEY_KP_2,
  KP_3 = GLFW_KEY_KP_3,
  KP_4 = GLFW_KEY_KP_4,
  KP_5 = GLFW_KEY_KP_5,
  KP_6 = GLFW_KEY_KP_6,
  KP_7 = GLFW_KEY_KP_7,
  KP_8 = GLFW_KEY_KP_8,
  KP_9 = GLFW_KEY_KP_9,
  KP_DECIMAL = GLFW_KEY_KP_DECIMAL,
  KP_DIVIDE = GLFW_KEY_KP_DIVIDE,
  KP_MULTIPLY = GLFW_KEY_KP_MULTIPLY,
  KP_SUBTRACT = GLFW_KEY_KP_SUBTRACT,
  KP_ADD = GLFW_KEY_KP_ADD,
  KP_ENTER = GLFW_KEY_KP_ENTER,
  KP_EQUAL = GLFW_KEY_KP_EQUAL,
  LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
  LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL,
  LEFT_ALT = GLFW_KEY_LEFT_ALT,
  LEFT_SUPER = GLFW_KEY_LEFT_SUPER,
  RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,
  RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL,
  RIGHT_ALT = GLFW_KEY_RIGHT_ALT,
  RIGHT_SUPER = GLFW_KEY_RIGHT_SUPER,
  MENU = GLFW_KEY_MENU,
  LAST = GLFW_KEY_LAST
};

class InputManager : public Manager {
 public:
  virtual bool GetKey(KeyCode);
  virtual bool GetKeyUp(KeyCode);
  virtual bool GetKeyDown(KeyCode);
  virtual glm::vec2 GetMousePosition();
  virtual void SetMousePosition(float, float);

  virtual void Initialize() override;
  virtual void Update() override;

 private:
  virtual GLFWwindow *GetWindow();
};

class NullInputManager : public InputManager {
 public:
   virtual bool GetKey(KeyCode) override { return false; };
   virtual bool GetKeyUp(KeyCode) override { return false; };
   virtual bool GetKeyDown(KeyCode) override { return false; };

   virtual glm::vec2 GetMousePosition() override
     { return glm::vec2(0.0f, 0.0f); };

   virtual void SetMousePosition(float, float) override {};
};
}  // namespace koma

#endif  // KOMA_CORE_INPUT_INPUT_MANAGER_HPP_
