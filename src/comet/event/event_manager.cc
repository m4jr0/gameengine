// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "event_manager.h"

namespace comet {
namespace event {
EventManager::EventManager(uindex queue_size) : event_queue_{queue_size} {}

void EventManager::Register(const EventListener& function,
                            stringid::StringId event_type) {
  std::scoped_lock<std::mutex> lock(mutex_);
  auto& listeners{listeners_[event_type]};
  listeners.emplace_back(function);
}

void EventManager::FireEventNow(std::unique_ptr<Event> event) {
  Dispatch(std::move(event));
}

void EventManager::FireEvent(std::unique_ptr<Event> event) {
  std::scoped_lock<std::mutex> lock(mutex_);
  event_queue_.push(std::move(event));
}

void EventManager::Dispatch(std::unique_ptr<Event> event) {
  const auto& listeners{listeners_[event->GetType()]};
  const auto event_pointer{event.get()};

  for (const auto& listener : listeners) {
    listener(*event_pointer);
  }
}

void EventManager::FireAllEvents() {
  std::scoped_lock<std::mutex> lock(mutex_);
  std::unique_ptr<Event> event;

  while (!event_queue_.empty()) {
    Dispatch(std::move(event_queue_.front()));
    event_queue_.pop();
  }
}
}  // namespace event
}  // namespace comet