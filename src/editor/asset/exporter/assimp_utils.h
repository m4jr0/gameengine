// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_ASSIMP_UTILS_H_
#define COMET_EDITOR_ASSET_EXPORTER_ASSIMP_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "assimp/matrix4x4.h"
#include "assimp/quaternion.h"
#include "assimp/scene.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/math/quaternion.h"
#include "comet/math/vector.h"

namespace comet {
namespace editor {
namespace asset {
math::Vec3 ToVec3(const aiVector3D& assimp_vec);
math::Quat ToQuat(const aiQuaternion& assimp_quat);
math::Mat4x4 ToMat4x4(const aiMatrix4x4& assimp_mat);
aiMatrix4x4 GenerateGlobalTransform(const aiNode* node);
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_ASSIMP_UTILS_H_
