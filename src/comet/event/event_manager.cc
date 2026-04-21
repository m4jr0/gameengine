// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "event_manager.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/logger/logging.h"
#include "comet/core/memory/memory.h"
#include "comet/math/math_scalar.h"

namespace comet {
namespace event {
#ifdef COMET_DEBUG
namespace internal {
constexpr usize kMaxLoggedLeakedEventTypeCount{8};
constexpr usize kMaxLoggedLeakedListenerCount{8};

static void LogLeakedListenerPreview(stringid::StringId event_type,
                                     const Array<EventListener>& listeners) {
  const auto listener_count{listeners.GetSize()};

  if (listener_count == 0) {
    return;
  }

  const auto logged_count{
      math::Min(listener_count, kMaxLoggedLeakedListenerCount)};

  for (usize i{0}; i < logged_count; ++i) {
    COMET_LOG_WARNING(LoggerType::Event, "event::EventManager::OnShutdown",
                      "leaked event listener", "event_type", event_type,
                      "listener_id", listeners[i].id, "listener_index", i);
  }

  if (listener_count > kMaxLoggedLeakedListenerCount) {
    COMET_LOG_WARNING(LoggerType::Event, "event::EventManager::OnShutdown",
                      "leaked event listener logging truncated", "event_type",
                      event_type, "logged_listener_count", logged_count,
                      "remaining_listener_count",
                      listener_count - logged_count);
  }
}
}  // namespace internal
#endif  // COMET_DEBUG

EventManager& EventManager::Get() {
  static EventManager singleton{};
  return singleton;
}

EventManager::EventManager()
    : listener_allocator_{
          math::Max(
              sizeof(Pair<stringid::StringId, Array<EventListener>>),
              math::Max(sizeof(EventListener),
                        sizeof(Pair<EventListenerId, stringid::StringId>))),
          4096, memory::kEngineMemoryTagEvent} {}

EventListenerId EventManager::Register(const Callback& function,
                                       stringid::StringId event_type) {
  auto& listeners{listeners_[event_type]};
  const auto id{listener_id_counter_++};
  listeners.EmplaceBack(id, function);
  id_event_type_map_[id] = event_type;
  return id;
}

void EventManager::Unregister(EventListenerId id) {
  COMET_ASSERT(id_event_type_map_.IsContained(id),
               "event::EventManager::Unregister", "event type not found",
               "listener_id", id);

  auto& listeners{listeners_[id_event_type_map_.Get(id)]};
  auto found_index{kInvalidIndex};

  for (usize i{0}; i < listeners.GetSize(); ++i) {
    if (listeners[i].id == id) {
      found_index = i;
      break;
    }
  }

  COMET_ASSERT(found_index != kInvalidIndex, "event::EventManager::Unregister",
               "listener not found", "listener_id", id);

  listeners.RemoveFromPos(listeners.begin() + found_index);
  id_event_type_map_.Remove(id);
}

void EventManager::FireEventNow(EventPtr event) const {
  Dispatch(std::move(event));
}

void EventManager::FireEvent(EventPtr event) { Add(std::move(event)); }

void EventManager::FireAllEvents() {
  EventPtr event;

  while (event_queue_.TryPop(event)) {
    Dispatch(std::move(event));
  }
}

void EventManager::OnInitialize() {
  COMET_ASSERT(max_event_count_ > 0, "event::EventManager::OnInitialize",
               "max event count is invalid", "max_event_count",
               max_event_count_);

  listener_allocator_.Initialize();

  listeners_ = EventListeners{&listener_allocator_};
  id_event_type_map_ = IdEventTypeMap{&listener_allocator_};
  event_queue_ = EventQueue{&event_queue_allocator_, max_event_count_};
}

void EventManager::OnShutdown() {
  if (!id_event_type_map_.IsEmpty()) {
    COMET_LOG_WARNING(LoggerType::Event, "event::EventManager::OnShutdown",
                      "registered listeners still exist", "listener_count",
                      id_event_type_map_.GetEntryCount());
  }

#ifdef COMET_DEBUG
  usize logged_event_type_count{0};
  usize remaining_event_type_count{0};

  for (const auto& listener_entry : listeners_) {
    if (listener_entry.value.IsEmpty()) {
      continue;
    }

    if (logged_event_type_count < internal::kMaxLoggedLeakedEventTypeCount) {
      COMET_LOG_WARNING(LoggerType::Event, "event::EventManager::OnShutdown",
                        "listeners still registered for event type",
                        "event_type", listener_entry.key, "listener_count",
                        listener_entry.value.GetSize());

      internal::LogLeakedListenerPreview(listener_entry.key,
                                         listener_entry.value);
      ++logged_event_type_count;
    } else {
      ++remaining_event_type_count;
    }
  }

  if (remaining_event_type_count > 0) {
    COMET_LOG_WARNING(LoggerType::Event, "event::EventManager::OnShutdown",
                      "leaked event type logging truncated",
                      "logged_event_type_count", logged_event_type_count,
                      "remaining_event_type_count", remaining_event_type_count);
  }
#endif  // COMET_DEBUG

  listeners_ = {};
  id_event_type_map_ = {};
  event_queue_ = {};
  listener_allocator_.Destroy();
}

void EventManager::Add(EventPtr event) { event_queue_.Push(std::move(event)); }

void EventManager::Dispatch(EventPtr event) const {
  auto* listeners{listeners_.TryGet(event->GetType())};

  if (listeners == nullptr) {
    return;
  }

  const auto event_pointer{event.get()};

  for (const auto& listener : *listeners) {
    listener.callback(*event_pointer);
  }
}
}  // namespace event
}  // namespace comet