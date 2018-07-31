// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_COMPONENT_HPP_
#define KOMA_CORE_COMPONENT_HPP_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace koma {
class Component {
 public:
  virtual ~Component() {};
  virtual void Update(double);
  virtual void FixedUpdate();
  virtual void Initialize();

  const boost::uuids::uuid kId() const;

 private:
  const boost::uuids::uuid kId_ = boost::uuids::random_generator()();
};
};  // namespace koma

#endif  // KOMA_CORE_COMPONENT_HPP_
