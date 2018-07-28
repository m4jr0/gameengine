// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_HPP_
#define KOMA_CORE_GAME_OBJECT_HPP_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>

namespace koma {
class GameObject {
 public:
  virtual ~GameObject() {};
  virtual void Update(double);
  virtual void FixedUpdate();
  const boost::uuids::uuid id() const;

 private:
    boost::uuids::uuid id_ = boost::uuids::random_generator()();
};
}; //  namespace koma

#endif //  KOMA_CORE_GAME_OBJECT_HPP_
