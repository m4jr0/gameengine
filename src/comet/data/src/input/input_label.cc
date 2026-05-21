// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/input/input_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace input {
const schar* GetMouseCursorModeLabel(MouseCursorMode mode) {
  switch (mode) {
    case MouseCursorMode::Unknown:
      return "unknown";
    case MouseCursorMode::Normal:
      return "normal";
    case MouseCursorMode::Disabled:
      return "disabled";
    default:
      return kUnknownLabel;
  }
}

const schar* GetMouseButtonLabel(MouseButton button) {
  switch (button) {
    case MouseButton::Unknown:
      return "unknown";
    case MouseButton::Left:
      return "left";
    case MouseButton::Right:
      return "right";
    case MouseButton::Middle:
      return "middle";
    case MouseButton::Other1:
      return "other_1";
    case MouseButton::Other2:
      return "other_2";
    case MouseButton::Other3:
      return "other_3";
    case MouseButton::Other4:
      return "other_4";
    case MouseButton::Other5:
      return "other_5";
    default:
      return kUnknownLabel;
  }
}

const schar* GetActionLabel(Action action) {
  switch (action) {
    case Action::Unknown:
      return "unknown";
    case Action::Press:
      return "press";
    case Action::Release:
      return "release";
    default:
      return kUnknownLabel;
  }
}

const schar* GetKeyCodeLabel(KeyCode key) {
  switch (key) {
    case KeyCode::Unknown:
      return "unknown";

    case KeyCode::Space:
      return "space";
    case KeyCode::Apostrophe:
      return "apostrophe";
    case KeyCode::Comma:
      return "comma";
    case KeyCode::Minus:
      return "minus";
    case KeyCode::Period:
      return "period";
    case KeyCode::Slash:
      return "slash";

    case KeyCode::Zero:
      return "0";
    case KeyCode::One:
      return "1";
    case KeyCode::Two:
      return "2";
    case KeyCode::Three:
      return "3";
    case KeyCode::Four:
      return "4";
    case KeyCode::Five:
      return "5";
    case KeyCode::Six:
      return "6";
    case KeyCode::Seven:
      return "7";
    case KeyCode::Eight:
      return "8";
    case KeyCode::Nine:
      return "9";

    case KeyCode::Semicolon:
      return "semicolon";
    case KeyCode::Equal:
      return "equal";

    case KeyCode::A:
      return "a";
    case KeyCode::B:
      return "b";
    case KeyCode::C:
      return "c";
    case KeyCode::D:
      return "d";
    case KeyCode::E:
      return "e";
    case KeyCode::F:
      return "f";
    case KeyCode::G:
      return "g";
    case KeyCode::H:
      return "h";
    case KeyCode::I:
      return "i";
    case KeyCode::J:
      return "j";
    case KeyCode::K:
      return "k";
    case KeyCode::L:
      return "l";
    case KeyCode::M:
      return "m";
    case KeyCode::N:
      return "n";
    case KeyCode::O:
      return "o";
    case KeyCode::P:
      return "p";
    case KeyCode::Q:
      return "q";
    case KeyCode::R:
      return "r";
    case KeyCode::S:
      return "s";
    case KeyCode::T:
      return "t";
    case KeyCode::U:
      return "u";
    case KeyCode::V:
      return "v";
    case KeyCode::W:
      return "w";
    case KeyCode::X:
      return "x";
    case KeyCode::Y:
      return "y";
    case KeyCode::Z:
      return "z";

    case KeyCode::LeftBracket:
      return "left_bracket";
    case KeyCode::Backslash:
      return "backslash";
    case KeyCode::RightBracket:
      return "right_bracket";
    case KeyCode::GraveAccent:
      return "grave_accent";
    case KeyCode::World1:
      return "world_1";
    case KeyCode::World2:
      return "world_2";

    case KeyCode::Escape:
      return "escape";
    case KeyCode::Enter:
      return "enter";
    case KeyCode::Tab:
      return "tab";
    case KeyCode::Backspace:
      return "backspace";
    case KeyCode::Insert:
      return "insert";
    case KeyCode::Delete:
      return "delete";

    case KeyCode::Right:
      return "right";
    case KeyCode::Left:
      return "left";
    case KeyCode::Down:
      return "down";
    case KeyCode::Up:
      return "up";

    case KeyCode::PageUp:
      return "page_up";
    case KeyCode::PageDown:
      return "page_down";
    case KeyCode::Home:
      return "home";
    case KeyCode::End:
      return "end";

    case KeyCode::CapsLock:
      return "caps_lock";
    case KeyCode::ScrollLock:
      return "scroll_lock";
    case KeyCode::NumLock:
      return "num_lock";
    case KeyCode::PrintScreen:
      return "print_screen";
    case KeyCode::Pause:
      return "pause";

    case KeyCode::F1:
      return "f1";
    case KeyCode::F2:
      return "f2";
    case KeyCode::F3:
      return "f3";
    case KeyCode::F4:
      return "f4";
    case KeyCode::F5:
      return "f5";
    case KeyCode::F6:
      return "f6";
    case KeyCode::F7:
      return "f7";
    case KeyCode::F8:
      return "f8";
    case KeyCode::F9:
      return "f9";
    case KeyCode::F10:
      return "f10";
    case KeyCode::F11:
      return "f11";
    case KeyCode::F12:
      return "f12";
    case KeyCode::F13:
      return "f13";
    case KeyCode::F14:
      return "f14";
    case KeyCode::F15:
      return "f15";
    case KeyCode::F16:
      return "f16";
    case KeyCode::F17:
      return "f17";
    case KeyCode::F18:
      return "f18";
    case KeyCode::F19:
      return "f19";
    case KeyCode::F20:
      return "f20";
    case KeyCode::F21:
      return "f21";
    case KeyCode::F22:
      return "f22";
    case KeyCode::F23:
      return "f23";
    case KeyCode::F24:
      return "f24";
    case KeyCode::F25:
      return "f25";

    case KeyCode::KeyPad0:
      return "kp_0";
    case KeyCode::KeyPad1:
      return "kp_1";
    case KeyCode::KeyPad2:
      return "kp_2";
    case KeyCode::KeyPad3:
      return "kp_3";
    case KeyCode::KeyPad4:
      return "kp_4";
    case KeyCode::KeyPad5:
      return "kp_5";
    case KeyCode::KeyPad6:
      return "kp_6";
    case KeyCode::KeyPad7:
      return "kp_7";
    case KeyCode::KeyPad8:
      return "kp_8";
    case KeyCode::KeyPad9:
      return "kp_9";
    case KeyCode::KeyPadDecimal:
      return "kp_decimal";
    case KeyCode::KeyPadDivide:
      return "kp_divide";
    case KeyCode::KeyPadMultiply:
      return "kp_multiply";
    case KeyCode::KeyPadSubtract:
      return "kp_subtract";
    case KeyCode::KeyPadAdd:
      return "kp_add";
    case KeyCode::KeyPadEnter:
      return "kp_enter";
    case KeyCode::KeyPadEqual:
      return "kp_equal";

    case KeyCode::LeftShift:
      return "left_shift";
    case KeyCode::LeftControl:
      return "left_control";
    case KeyCode::LeftAlt:
      return "left_alt";
    case KeyCode::LeftSuper:
      return "left_super";
    case KeyCode::RightShift:
      return "right_shift";
    case KeyCode::RightControl:
      return "right_control";
    case KeyCode::RightAlt:
      return "right_alt";
    case KeyCode::RightSuper:
      return "right_super";

    case KeyCode::Menu:
      return "menu";

    default:
      return kUnknownLabel;
  }
}
}  // namespace input
}  // namespace comet
