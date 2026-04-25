// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "component.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace entity {
HashValue GenerateHash(const ComponentTypeDescr& descr) {
  return comet::GenerateHash(descr.id);
}

const ComponentTypeDescrHashLogic::Hashable&
ComponentTypeDescrHashLogic::GetHashable(const Value& value) {
  return value;
}

HashValue ComponentTypeDescrHashLogic::Hash(const Hashable& hashable) {
  return GenerateHash(hashable);
}

bool ComponentTypeDescrHashLogic::AreEqual(const Hashable& a,
                                           const Hashable& b) {
  return a.id == b.id;
}
}  // namespace entity
}  // namespace comet
