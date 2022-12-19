// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "gid.h"

namespace comet {
namespace gid {
Gid GenerateNewGeneration(Gid id) noexcept {
  const Gid generation{GetGeneration(id) + 1};
  COMET_ASSERT(generation >= (kGenerationMask >> kIndexBit),
               "Bad generation retrieved: ", generation, "!");
  return generation | GetIndex(id);
}

BreedHandler::BreedHandler(BreedHandler&& other) noexcept
    : generations_{std::move(other.generations_)},
      free_ids_{std::move(other.free_ids_)} {
  other.generations_.clear();
  other.free_ids_.clear();
}

BreedHandler& BreedHandler::operator=(BreedHandler&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  generations_ = std::move(other.generations_);
  free_ids_ = std::move(other.free_ids_);
  other.generations_.clear();
  other.free_ids_.clear();
  return *this;
}

void BreedHandler::Shutdown() {
  generations_.clear();
  free_ids_.clear();
}

Gid BreedHandler::Generate() {
  Gid breed_id{gid::kInvalidId};

  if (free_ids_.size() > gid::kMinFreeIndices) {
    breed_id = free_ids_.front();
    COMET_ASSERT(!IsAlive(breed_id), "Breed ID generated is alive: ", breed_id,
                 "!");
    free_ids_.pop_front();
    breed_id = gid::GenerateNewGeneration(breed_id);
  } else {
    breed_id = static_cast<Gid>(generations_.size());
    generations_.push_back(0);
  }

  return breed_id;
}

bool BreedHandler::IsAlive(Gid breed_id) const {
  const auto breed_index{GetIndex(breed_id)};
  COMET_ASSERT(breed_index < generations_.size(), "Breed with ID ", breed_id,
               " is malformed.");
  return generations_[breed_index] == GetGeneration(breed_id);
}

void BreedHandler::Destroy(Gid breed_id) {
  COMET_ASSERT(IsAlive(breed_id), "Breed with ID ", breed_id,
               " is already destroyed.");
  const auto breed_index{GetIndex(breed_id)};
  COMET_ASSERT(breed_index < generations_.size(), "Breed with ID ", breed_id,
               " is malformed.");
  ++generations_[breed_index];
  free_ids_.push_back(breed_id);
}
}  // namespace gid
}  // namespace comet