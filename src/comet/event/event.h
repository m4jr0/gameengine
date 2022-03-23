// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_EVENT_EVENT_H_
#define COMET_COMET_EVENT_EVENT_H_

#define COMET_EVENT_BIND_FUNCTION(function) \
  [this](const comet::event::Event& event) { return this->function(event); }

#include "comet_precompile.h"

namespace comet {
namespace event {
// Base event class to inherit from when adding a new event.
// Here is an implementation example:
// In the .h file:
//   class ExplosionEvent : public Event {
//    public:
//     static const core::StringId kStaticType_;
//     ExplosionEvent(const unsigned int, const unsigned int);
//     ExplosionEvent(const ExplosionEvent&) = default;
//     ExplosionEvent(ExplosionEvent&&) noexcept = default;
//     ExplosionEvent& operator=(const ExplosionEvent&) = default;
//     ExplosionEvent& operator=(ExplosionEvent&&) noexcept = default;
//     virtual ~ExplosionEvent() = default;
//
//     virtual const core::StringId& GetType() const noexcept;
//     unsigned int GetRadius() const noexcept;
//     unsigned int GetDamage() const noexcept;
//
//    private:
//     // For performance purposes, we do not want to generate the string ID
//     // every time. The static type CANNOT be inline.
//
//     unsigned int radius_ = 0;
//     unsigned int damage_ = 0;
//   };

// In the .cc file:
//   ExplosionEvent::kStaticType_ = core::StringId::Generate("event_explosion");
//
//   ExplosionEvent::ExplosionEvent(const unsigned int radius,
//                                  const unsigned int damage) {
//     radius_ = radius;
//     damage_ = damage;
//   }
//
//   unsigned int ExplosionEvent::GetRadius() const noexcept { return radius_; }
//
//   unsigned int ExplosionEvent::GetDamage() const noexcept { return damage_; }
//
//   const core::StringId& ExplosionEvent::GetType() const noexcept {
//     return kStaticType_;
//   };

class Event {
 public:
  Event() = default;
  Event(const Event&) = default;
  Event(Event&&) noexcept = default;
  Event& operator=(const Event&) = default;
  Event& operator=(Event&&) noexcept = default;
  virtual ~Event() = default;

  template <typename T, typename... Targs>
  static std::unique_ptr<T> Create(Targs... args) {
    if (!std::is_base_of<Event, T>::value) {
      return nullptr;
    }

    return std::make_unique<T>(args...);
  }

  virtual const core::StringId& GetType() const noexcept = 0;
};
}  // namespace event
}  // namespace comet

#endif  // COMET_COMET_EVENT_EVENT_H_
