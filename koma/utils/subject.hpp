// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_UTILS_SUBJECT_HPP_
#define KOMA_UTILS_SUBJECT_HPP_

#include <string>
#include <unordered_set>

#include "observer.hpp"

namespace koma {
class Subject {
public:
  virtual void AddObserver(Observer*);
  virtual void RemoveObserver(Observer*);
  virtual void NotifyObservers(std::string);

private:
  std::unordered_set<Observer*> observers_;
};
}  // namespace koma

#endif  // KOMA_UTILS_SUBJECT_HPP_
