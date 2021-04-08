// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_EVENT_EVENT_HPP_
#define COMET_CORE_EVENT_EVENT_HPP_

#define COMET_EVENT_BIND_FUNCTION(fn)                       \
  [this](auto&&... args) -> decltype(auto) {                \
    return this->fn(std::forward<decltype(args)>(args)...); \
  }

#include "comet_precompile.h"

namespace comet {
namespace event {
enum class Type {
  Unknown = 0,
  WindowResize,
};

enum Category {
  kUnknown = 1 << 0,
  kEngine = 1 << 1,
  kInput = 1 << 2,
  kKeyboard = 1 << 3,
  kMouse = 1 << 4,
};

class Event {
 public:
  static constexpr Type kStaticType_ = Type::Unknown;

  virtual ~Event() = default;

  template <typename T, typename... Targs>
  static std::unique_ptr<T> Create(Targs... args) {
    if (!std::is_base_of<Event, T>::value) {
      return nullptr;
    }

    auto event = std::make_unique<T>(args...);
    event->type_ = T::kStaticType_;

    return event;
  }

  Type GetType() { return type_; }
  virtual int GetCategoryFlags() const = 0;
  bool IsInCategory(Category category) const;

 protected:
  Event() = default;

  Type type_ = Type::Unknown;
};

class SpecificEvent : public Event {
 public:
  static constexpr Type kStaticType_ = Type::WindowResize;

  SpecificEvent(const std::string& data) { data_ = data; }

  const std::string& GetString() const { return data_; }

  virtual int GetCategoryFlags() const override { return Category::kUnknown; }

 private:
  std::string data_ = "default";
};
}  // namespace event
}  // namespace comet

#endif  // COMET_CORE_EVENT_EVENT_HPP_
