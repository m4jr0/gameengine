// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "assimp_utils.h"

namespace comet {
namespace editor {
namespace asset {
math::Vec3 ToVec3(const aiVector3D& assimp_vec) {
  return math::Vec3{assimp_vec.x, assimp_vec.y, assimp_vec.z};
}

math::Quat ToQuat(const aiQuaternion& assimp_quat) {
  return math::Quat{assimp_quat.w, assimp_quat.x, assimp_quat.y, assimp_quat.z};
}

math::Mat4x4 ToMat4x4(const aiMatrix4x4& assimp_mat) {
  math::Mat4x4 mat{};
  mat[0][0] = assimp_mat.a1;
  mat[1][0] = assimp_mat.a2;
  mat[2][0] = assimp_mat.a3;
  mat[3][0] = assimp_mat.a4;
  mat[0][1] = assimp_mat.b1;
  mat[1][1] = assimp_mat.b2;
  mat[2][1] = assimp_mat.b3;
  mat[3][1] = assimp_mat.b4;
  mat[0][2] = assimp_mat.c1;
  mat[1][2] = assimp_mat.c2;
  mat[2][2] = assimp_mat.c3;
  mat[3][2] = assimp_mat.c4;
  mat[0][3] = assimp_mat.d1;
  mat[1][3] = assimp_mat.d2;
  mat[2][3] = assimp_mat.d3;
  mat[3][3] = assimp_mat.d4;
  return mat;
}

aiMatrix4x4 GenerateGlobalTransform(const aiNode* node) {
  auto transform{node->mTransformation};
  const auto* current{node->mParent};

  while (current != nullptr) {
    transform = current->mTransformation * transform;
    current = current->mParent;
  }

  return transform;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
