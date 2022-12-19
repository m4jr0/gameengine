// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "event_manager.h"

namespace comet {
namespace event {
EventManager::EventManager(uindex queue_size) : event_queue_{queue_size} {}

void EventManager::Shutdown() {
  listeners_.clear();
  event_queue_.clear();
  Manager::Shutdown();
}

void EventManager::Register(const EventListener& function,
                            stringid::StringId event_type) {
  std::scoped_lock<std::mutex> lock(mutex_);
  auto& listeners{listeners_[event_type]};
  listeners.push_back(function);
}

void EventManager::FireEventNow(std::unique_ptr<Event> event) const {
  Dispatch(std::move(event));
}

void EventManager::FireEvent(std::unique_ptr<Event> event) {
  std::scoped_lock<std::mutex> lock(mutex_);
  event_queue_.push(std::move(event));
}

void EventManager::FireAllEvents() {
  std::scoped_lock<std::mutex> lock(mutex_);
  std::unique_ptr<Event> event;

  while (!event_queue_.empty()) {
    Dispatch(std::move(event_queue_.front()));
    event_queue_.pop();
  }
}

void EventManager::Dispatch(std::unique_ptr<Event> event) const {
  const auto& listeners{listeners_.at(event->GetType())};
  const auto event_pointer{event.get()};

  for (const auto& listener : listeners) {
    listener(*event_pointer);
  }
}
}  // namespace event
}  // namespace comet