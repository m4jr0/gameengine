// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "subject.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include <debug_windows.hpp>
#endif  // _WIN32

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
}  // namespace koma
