// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allow debugging memory leaks.
#include "../debug.hpp"

#include "subject.hpp"

namespace koma {
void Subject::AddObserver(Observer *observer) {
  this->observers_.insert(observer);
}

void Subject::RemoveObserver(Observer *observer) {
  this->observers_.erase(observer);
}

void Subject::NotifyObservers(std::string event) {
  for (Observer* observer : this->observers_) {
    observer->ReceiveEvent(event);
  }
}
};  // namespace koma
