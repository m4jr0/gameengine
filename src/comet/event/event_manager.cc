// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "event_manager.h"

namespace comet {
namespace event {
EventManager::EventManager(std::size_t queue_size)
    : event_queue_(queue_size) {}

void EventManager::Dispatch(std::unique_ptr<Event> event) {
  const int event_type = static_cast<int>(event->GetType());
  const auto& listeners = listeners_[event_type];
  const auto event_pointer = event.get();

  for (const auto& listener : listeners) {
    listener->Call(event_pointer);
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