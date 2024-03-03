// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_INPUT_INPUT_H_
#define COMET_COMET_INPUT_INPUT_H_

#include "comet_precompile.h"

#define GLFW_INCLUDE_NONE

#include "GLFW/glfw3.h"

namespace comet {
namespace input {
// Here, we wrap the GLFW "input manager" to make it easier if we want to
// change it, some day.
enum class KeyCode {
  Unknown = GLFW_KEY_UNKNOWN,
  Space = GLFW_KEY_SPACE,
  Apostrophe = GLFW_KEY_APOSTROPHE,
  Comma = GLFW_KEY_COMMA,
  Minus = GLFW_KEY_MINUS,
  Period = GLFW_KEY_PERIOD,
  Slash = GLFW_KEY_SLASH,
  Zero = GLFW_KEY_0,
  One = GLFW_KEY_1,
  Two = GLFW_KEY_2,
  Three = GLFW_KEY_3,
  Four = GLFW_KEY_4,
  Five = GLFW_KEY_5,
  Six = GLFW_KEY_6,
  Seven = GLFW_KEY_7,
  Eight = GLFW_KEY_8,
  Nine = GLFW_KEY_9,
  Semicolon = GLFW_KEY_SEMICOLON,
  Equal = GLFW_KEY_EQUAL,
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
  LeftBracket = GLFW_KEY_LEFT_BRACKET,
  Backslash = GLFW_KEY_BACKSLASH,
  RightBracket = GLFW_KEY_RIGHT_BRACKET,
  GraveAcent = GLFW_KEY_GRAVE_ACCENT,
  World1 = GLFW_KEY_WORLD_1,
  World2 = GLFW_KEY_WORLD_2,
  Escape = GLFW_KEY_ESCAPE,
  Enter = GLFW_KEY_ENTER,
  Tab = GLFW_KEY_TAB,
  Backspace = GLFW_KEY_BACKSPACE,
  Insert = GLFW_KEY_INSERT,
  Del = GLFW_KEY_DELETE,
  Right = GLFW_KEY_RIGHT,
  Left = GLFW_KEY_LEFT,
  Down = GLFW_KEY_DOWN,
  Up = GLFW_KEY_UP,
  PageUp = GLFW_KEY_PAGE_UP,
  PageDown = GLFW_KEY_PAGE_DOWN,
  Home = GLFW_KEY_HOME,
  End = GLFW_KEY_END,
  CapsLock = GLFW_KEY_CAPS_LOCK,
  ScrollLock = GLFW_KEY_SCROLL_LOCK,
  NumLock = GLFW_KEY_NUM_LOCK,
  PrintScreen = GLFW_KEY_PRINT_SCREEN,
  Pause = GLFW_KEY_PAUSE,
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
  KP0 = GLFW_KEY_KP_0,
  KeyPad1 = GLFW_KEY_KP_1,
  KeyPad2 = GLFW_KEY_KP_2,
  KeyPad3 = GLFW_KEY_KP_3,
  KeyPad4 = GLFW_KEY_KP_4,
  KeyPad5 = GLFW_KEY_KP_5,
  KeyPad6 = GLFW_KEY_KP_6,
  KeyPad7 = GLFW_KEY_KP_7,
  KeyPad8 = GLFW_KEY_KP_8,
  KeyPad9 = GLFW_KEY_KP_9,
  KeyPadDecimal = GLFW_KEY_KP_DECIMAL,
  KeyPadDivide = GLFW_KEY_KP_DIVIDE,
  KeyPadMultiply = GLFW_KEY_KP_MULTIPLY,
  KeyPadSubtract = GLFW_KEY_KP_SUBTRACT,
  KeyPadAdd = GLFW_KEY_KP_ADD,
  KeyPadEnter = GLFW_KEY_KP_ENTER,
  KeyPadEqual = GLFW_KEY_KP_EQUAL,
  LeftShift = GLFW_KEY_LEFT_SHIFT,
  LeftControl = GLFW_KEY_LEFT_CONTROL,
  LeftAlt = GLFW_KEY_LEFT_ALT,
  LeftSuper = GLFW_KEY_LEFT_SUPER,
  RightShift = GLFW_KEY_RIGHT_SHIFT,
  RightControl = GLFW_KEY_RIGHT_CONTROL,
  RightAlt = GLFW_KEY_RIGHT_ALT,
  RightSuper = GLFW_KEY_RIGHT_SUPER,
  Menu = GLFW_KEY_MENU,
  Last = GLFW_KEY_LAST
};

enum class MouseButton {
  Unknown = -1,
  Left = GLFW_MOUSE_BUTTON_1,
  Right = GLFW_MOUSE_BUTTON_2,
  Middle = GLFW_MOUSE_BUTTON_3,
  Last = GLFW_MOUSE_BUTTON_8,
  Other1 = GLFW_MOUSE_BUTTON_4,
  Other2 = GLFW_MOUSE_BUTTON_5,
  Other3 = GLFW_MOUSE_BUTTON_6,
  Other4 = GLFW_MOUSE_BUTTON_7
};

enum class Action { Unknown = -1, Press = GLFW_PRESS, Release = GLFW_RELEASE };

enum Mods {
  Empty = 0,
  Shift = GLFW_MOD_SHIFT,
  Control = GLFW_MOD_CONTROL,
  Alt = GLFW_MOD_ALT,
  Super = GLFW_MOD_SUPER,
  CapsLock = GLFW_MOD_CAPS_LOCK,
  NumLock = GLFW_MOD_NUM_LOCK
};

using ScanCode = s32;
}  // namespace input
}  // namespace comet

#endif  // COMET_COMET_INPUT_INPUT_H_
