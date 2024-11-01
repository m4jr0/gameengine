// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_EVENT_EVENT_H_
#define COMET_COMET_EVENT_EVENT_H_

#include <atomic>

#include "comet/core/essentials.h"
#include "comet/core/frame/stl/two_frame_allocator.h"
#include "comet/core/logger.h"
#include "comet/core/type/stl_types.h"
#include "comet/core/type/string_id.h"

#define COMET_EVENT_BIND_FUNCTION(function) \
  [this](const comet::event::Event& event) { return this->function(event); }

namespace comet {
namespace event {
// Base event class to inherit from when adding a new event.
// Here is an implementation example:
// In the .h file:
//   class ExplosionEvent : public Event {
//    public:
//     static const stringid::StringId kStaticType_;
//     ExplosionEvent(const u32 radius, const u32 damage);
//     ExplosionEvent(const ExplosionEvent&) = default;
//     ExplosionEvent(ExplosionEvent&&) noexcept = default;
//     ExplosionEvent& operator=(const ExplosionEvent&) = default;
//     ExplosionEvent& operator=(ExplosionEvent&&) noexcept = default;
//     ~ExplosionEvent() = default;
//
//     const stringid::StringId& GetType() const noexcept;
//     u32 GetRadius() const noexcept;
//     u32 GetDamage() const noexcept;
//
//    private:
//     // For performance purposes, we do not want to generate the string ID
//     // every time. The static type CANNOT be inline.
//
//     u32 radius_{0};
//     u32 damage_{0};
//   };

// In the .cc file:
//   ExplosionEvent::kStaticType_ = COMET_STRING_ID("event_explosion");
//
//    ExplosionEvent::ExplosionEvent(const u32 radius,
//                                   const u32 damage)
//        : radius_{radius}, damage_{damage} {}
//
//   u32 ExplosionEvent::GetRadius() const noexcept { return radius_; }
//
//   u32 ExplosionEvent::GetDamage() const noexcept { return damage_; }
//
//   const stringid::StringId& ExplosionEvent::GetType() const noexcept {
//     return kStaticType_;
//   };

using SequenceNumber = u64;

class Event {
 public:
  Event();
  Event(const Event&) = default;
  Event(Event&&) noexcept = default;
  Event& operator=(const Event&) = default;
  Event& operator=(Event&&) noexcept = default;
  virtual ~Event() = default;

  SequenceNumber GetSequenceNumber() const noexcept;
  virtual stringid::StringId GetType() const noexcept = 0;

 private:
  static_assert(
      std::atomic<SequenceNumber>::is_always_lock_free,
      "std::atomic<SequenceNumber> needs to be always lock-free. Unsupported "
      "architecture");
  static inline std::atomic<SequenceNumber> sequence_number_count_{0};
  SequenceNumber sequence_number_{0};
};

using EventPointer = custom_unique_ptr<comet::event::Event>;

// Function that creates and returns a custom unique pointer using custom
// allocator and deleter
template <typename T, typename... Args>
EventPointer GenerateEvent(Args&&... args) {
  frame::two_cycle_frame_allocator<T> allocator{};
  auto* p{allocator.allocate_one()};

  // No need to deallocate if an exception occurs: the temporary allocator will
  // be flushed by the end of the frame following the current one.
  EventPointer event(p, [](Event* p) { p->~Event(); });
  new (event.get()) T{std::forward<Args>(args)...};
  return event;
}
}  // namespace event
}  // namespace comet

#endif  // COMET_COMET_EVENT_EVENT_H_
