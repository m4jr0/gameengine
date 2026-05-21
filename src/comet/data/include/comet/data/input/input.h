// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_INPUT_INPUT_H_
#define COMET_DATA_INPUT_INPUT_H_

#include "comet/core/essentials.h"

namespace comet {
namespace input {
using ScanCode = s32;
using Mods = u32;

enum class Action : s32 {
  Unknown = -1,
  Release = 0,
  Press = 1,
  Repeat = 2,
};

enum ModBits : Mods {
  kModBitsNone = 0x0,
  kModBitsShift = 0x1,
  kModBitsControl = 0x2,
  kModBitsAlt = 0x4,
  kModBitsSuper = 0x8,
};

enum class MouseCursorMode : s32 {
  Unknown = -1,
  Normal,
  Disabled,
};

enum class MouseButton : s32 {
  Unknown = -1,
  Left = 0,
  Right = 1,
  Middle = 2,
  Other1 = 3,
  Other2 = 4,
  Other3 = 5,
  Other4 = 6,
  Other5 = 7,
};

enum class KeyCode : s32 {
  Unknown = -1,

  Space = 32,
  Apostrophe = 39,
  Comma = 44,
  Minus = 45,
  Period = 46,
  Slash = 47,

  Zero = 48,
  One = 49,
  Two = 50,
  Three = 51,
  Four = 52,
  Five = 53,
  Six = 54,
  Seven = 55,
  Eight = 56,
  Nine = 57,

  Semicolon = 59,
  Equal = 61,

  A = 65,
  B = 66,
  C = 67,
  D = 68,
  E = 69,
  F = 70,
  G = 71,
  H = 72,
  I = 73,
  J = 74,
  K = 75,
  L = 76,
  M = 77,
  N = 78,
  O = 79,
  P = 80,
  Q = 81,
  R = 82,
  S = 83,
  T = 84,
  U = 85,
  V = 86,
  W = 87,
  X = 88,
  Y = 89,
  Z = 90,

  Escape = 256,
  Enter = 257,
  Tab = 258,
  Backspace = 259,
  Insert = 260,
  Delete = 261,
  Right = 262,
  Left = 263,
  Down = 264,
  Up = 265,

  LeftShift = 340,
  LeftControl = 341,
  LeftAlt = 342,
  LeftSuper = 343,
  RightShift = 344,
  RightControl = 345,
  RightAlt = 346,
  RightSuper = 347,
};

namespace internal {
constexpr s32 kKeyBaseOffset{32};
constexpr usize kKeyCount{316};
constexpr usize kMouseButtonCount{8};
}  // namespace internal
}  // namespace input
}  // namespace comet

#endif  // COMET_DATA_INPUT_INPUT_H_