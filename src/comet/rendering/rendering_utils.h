// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDERING_UTILS_H_
#define COMET_COMET_RENDERING_RENDERING_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/geometry/geometry_type.h"
#include "comet/math/bounding_volume.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/rendering/culling/frustum.h"
#include "comet/rendering/rendering_type.h"

namespace comet {
namespace rendering {
enum class ClipSpaceDepthRange : u8 { MinusOneToOne, ZeroToOne };

math::Vec3 ComputeStableUpVector(const math::Vec3& forward);

math::Mat4 LookAt(const math::Vec3& eye, const math::Vec3& center,
                  const math::Vec3& up);

math::Mat4 GeneratePerspectiveMatrix(f32 vertical_fov, f32 ratio, f32 z_near,
                                     f32 z_far,
                                     ClipSpaceDepthRange depth_range);
math::Mat4 GenerateOrthographicMatrix(f32 left, f32 right, f32 bottom, f32 top,
                                      f32 z_near, f32 z_far,
                                      ClipSpaceDepthRange depth_range);

math::Vec4 GenerateTangentWithSign(const math::Vec3& raw_normal,
                                   const math::Vec3& raw_tangent,
                                   const math::Vec3* raw_bitangent = nullptr);

DriverType GetDriverTypeFromStr(std::string_view str);
const schar* GetDriverTypeLabel(DriverType type);
DriverType GetDriverType();

bool IsMultithreading([[maybe_unused]] DriverType type);

AntiAliasingType GetAntiAliasingTypeFromStr(std::string_view str);

const schar* GetTextureTypeLabel(TextureType texture_type);
const schar* GetTextureFilterModeLabel(TextureFilterMode filter_mode);
const schar* GetTextureRepeatModeLabel(TextureRepeatMode repeat_mode);

Alignment GetScalarAlignment(ShaderVariableType type);
Alignment GetStd140Alignment(ShaderVariableType type);
Alignment GetStd430Alignment(ShaderVariableType type);

ShaderVariableSize GetShaderVariableTypeSize(ShaderVariableType type);

void SetName(ShaderNamedDescr& descr, const schar* name, usize name_len);
void SetName(ShaderFieldDescr& descr, const schar* name, usize name_len);
void SetName(ShaderBindingDescr& descr, const schar* name, usize name_len);
void SetName(ShaderPushConstantDescr& descr, const schar* name, usize name_len);
void SetName(ShaderDefineDescr& descr, const schar* name, usize name_len);
void SetValue(ShaderDefineDescr& descr, const schar* value, usize value_len);

void GenerateGeometry(const math::Aabb& aabb,
                      Array<geometry::SkinnedVertex>& vertices,
                      Array<geometry::Index>& indices, bool is_visible);

void GenerateGeometry(const Frustum& frustum,
                      Array<geometry::SkinnedVertex>& vertices,
                      Array<geometry::Index>& indices);

bool IsBufferBindingType(ShaderBindingType type);
bool IsImageBindingType(ShaderBindingType type);
bool IsSrgbTextureType(TextureType type);

u32 GetMipLevels(u32 width, u32 height);
u8 GetResolvedChannelCount(TextureFormat format, u8 fallback);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_RENDERING_UTILS_H_
