// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "data/render/utils/shader_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/logger/logging.h"

namespace comet {
namespace rendering {
Alignment GetScalarAlignment(ShaderVariableType type) {
  switch (type) {
    case ShaderVariableType::B32:
    case ShaderVariableType::S32:
    case ShaderVariableType::U32:
    case ShaderVariableType::F32:
      return 4;

    case ShaderVariableType::F64:
      return 8;

    case ShaderVariableType::B32Vec2:
    case ShaderVariableType::B32Vec3:
    case ShaderVariableType::B32Vec4:
    case ShaderVariableType::S32Vec2:
    case ShaderVariableType::S32Vec3:
    case ShaderVariableType::S32Vec4:
    case ShaderVariableType::U32Vec2:
    case ShaderVariableType::U32Vec3:
    case ShaderVariableType::U32Vec4:
    case ShaderVariableType::Vec2:
    case ShaderVariableType::Vec3:
    case ShaderVariableType::Vec4:
      return 4;

    case ShaderVariableType::F64Vec2:
    case ShaderVariableType::F64Vec3:
    case ShaderVariableType::F64Vec4:
      return 8;

    case ShaderVariableType::Mat2x2:
    case ShaderVariableType::Mat2x3:
    case ShaderVariableType::Mat2x4:
    case ShaderVariableType::Mat3x2:
    case ShaderVariableType::Mat3x3:
    case ShaderVariableType::Mat3x4:
    case ShaderVariableType::Mat4x2:
    case ShaderVariableType::Mat4x3:
    case ShaderVariableType::Mat4x4:
      return 4;

    default:
      return kInvalidAlignment;
  }
}

Alignment GetStd140Alignment(ShaderVariableType type) {
  switch (type) {
    case ShaderVariableType::B32:
    case ShaderVariableType::S32:
    case ShaderVariableType::U32:
    case ShaderVariableType::F32:
      return 4;

    case ShaderVariableType::F64:
      return 8;

    case ShaderVariableType::B32Vec2:
    case ShaderVariableType::S32Vec2:
    case ShaderVariableType::U32Vec2:
    case ShaderVariableType::Vec2:
      return 8;

    case ShaderVariableType::B32Vec3:
    case ShaderVariableType::B32Vec4:
    case ShaderVariableType::S32Vec3:
    case ShaderVariableType::S32Vec4:
    case ShaderVariableType::U32Vec3:
    case ShaderVariableType::U32Vec4:
    case ShaderVariableType::Vec3:
    case ShaderVariableType::Vec4:
      return 16;

    case ShaderVariableType::F64Vec2:
      return 16;

    case ShaderVariableType::F64Vec3:
    case ShaderVariableType::F64Vec4:
      return 32;

    case ShaderVariableType::Mat2x2:
    case ShaderVariableType::Mat2x3:
    case ShaderVariableType::Mat2x4:
    case ShaderVariableType::Mat3x2:
    case ShaderVariableType::Mat3x3:
    case ShaderVariableType::Mat3x4:
    case ShaderVariableType::Mat4x2:
    case ShaderVariableType::Mat4x3:
    case ShaderVariableType::Mat4x4:
      return 16;

    default:
      return kInvalidAlignment;
  }
}

Alignment GetStd430Alignment(ShaderVariableType type) {
  switch (type) {
    case ShaderVariableType::B32:
    case ShaderVariableType::S32:
    case ShaderVariableType::U32:
    case ShaderVariableType::F32:
      return 4;

    case ShaderVariableType::F64:
      return 8;

    case ShaderVariableType::B32Vec2:
    case ShaderVariableType::S32Vec2:
    case ShaderVariableType::U32Vec2:
    case ShaderVariableType::Vec2:
      return 8;

    case ShaderVariableType::B32Vec3:
    case ShaderVariableType::B32Vec4:
    case ShaderVariableType::S32Vec3:
    case ShaderVariableType::S32Vec4:
    case ShaderVariableType::U32Vec3:
    case ShaderVariableType::U32Vec4:
    case ShaderVariableType::Vec3:
    case ShaderVariableType::Vec4:
      return 16;

    case ShaderVariableType::F64Vec2:
      return 16;

    case ShaderVariableType::F64Vec3:
    case ShaderVariableType::F64Vec4:
      return 32;

    case ShaderVariableType::Mat2x2:
    case ShaderVariableType::Mat2x3:
    case ShaderVariableType::Mat2x4:
      return 8;

    case ShaderVariableType::Mat3x2:
    case ShaderVariableType::Mat3x3:
    case ShaderVariableType::Mat3x4:
    case ShaderVariableType::Mat4x2:
    case ShaderVariableType::Mat4x3:
    case ShaderVariableType::Mat4x4:
      return 16;

    default:
      return kInvalidAlignment;
  }
}

ShaderVariableSize GetShaderVariableTypeSize(ShaderVariableType type) {
  switch (type) {
    case ShaderVariableType::B32:
    case ShaderVariableType::S32:
    case ShaderVariableType::U32:
    case ShaderVariableType::F32:
      return 4;

    case ShaderVariableType::B32Vec2:
    case ShaderVariableType::S32Vec2:
    case ShaderVariableType::U32Vec2:
    case ShaderVariableType::Vec2:
      return 2 * 4;

    case ShaderVariableType::B32Vec3:
    case ShaderVariableType::S32Vec3:
    case ShaderVariableType::U32Vec3:
    case ShaderVariableType::Vec3:
      return 3 * 4;

    case ShaderVariableType::B32Vec4:
    case ShaderVariableType::S32Vec4:
    case ShaderVariableType::U32Vec4:
    case ShaderVariableType::Vec4:
      return 4 * 4;

    case ShaderVariableType::F64:
      return 8;

    case ShaderVariableType::F64Vec2:
      return 2 * 8;

    case ShaderVariableType::F64Vec3:
      return 3 * 8;

    case ShaderVariableType::F64Vec4:
      return 4 * 8;

    case ShaderVariableType::Mat2x2:
      return 2 * 2 * 4;

    case ShaderVariableType::Mat2x3:
    case ShaderVariableType::Mat3x2:
      return 2 * 3 * 4;

    case ShaderVariableType::Mat3x3:
      return 3 * 3 * 4;

    case ShaderVariableType::Mat2x4:
    case ShaderVariableType::Mat4x2:
      return 2 * 4 * 4;

    case ShaderVariableType::Mat3x4:
    case ShaderVariableType::Mat4x3:
      return 3 * 4 * 4;

    case ShaderVariableType::Mat4x4:
      return 4 * 4 * 4;

    case ShaderVariableType::Sampler:
    case ShaderVariableType::Image:
    case ShaderVariableType::Atomic:
    case ShaderVariableType::Unknown:
      return kInvalidShaderVariableSize;
  }

  return kInvalidShaderVariableSize;
}

namespace internal {
template <typename TDescr>
void SetShaderNameInternal(TDescr& descr, const schar* name, usize name_len) {
  COMET_ASSERT(name != nullptr, "rendering_shader_utils::SetShaderNameInternal",
               "name is null");

  descr.name_len = name_len;

  if (descr.name_len >= kShaderNameMaxLen) {
    COMET_LOG_WARNING(LoggerType::Rendering,
                      "rendering_shader_utils::internal::SetShaderNameInternal",
                      "shader name is too long and will be truncated",
                      "name_len", descr.name_len, "max_name_len",
                      kShaderNameMaxLen);

    descr.name_len = static_cast<usize>(kShaderNameMaxLen);
  }

  Copy(descr.name, name, descr.name_len);
  descr.name[descr.name_len] = '\0';
}
}  // namespace internal

void SetName(ShaderNamedDescr& descr, const schar* name, usize name_len) {
  internal::SetShaderNameInternal(descr, name, name_len);
}

void SetName(ShaderFieldDescr& descr, const schar* name, usize name_len) {
  internal::SetShaderNameInternal(descr, name, name_len);
}

void SetName(ShaderBindingDescr& descr, const schar* name, usize name_len) {
  internal::SetShaderNameInternal(descr, name, name_len);
}

void SetName(ShaderPushConstantDescr& descr, const schar* name,
             usize name_len) {
  internal::SetShaderNameInternal(descr, name, name_len);
}

void SetName(ShaderDefineDescr& descr, const schar* name, usize name_len) {
  COMET_ASSERT(name != nullptr, "rendering_shader_utils::SetName",
               "name is null");

  descr.name_len = name_len;

  if (descr.name_len >= kMaxShaderDefineNameLen) {
    COMET_LOG_WARNING(LoggerType::Rendering, "rendering_shader_utils::SetName",
                      "shader define name is too long and will be truncated",
                      "name_len", descr.name_len, "max_name_len",
                      kMaxShaderDefineNameLen);

    descr.name_len = static_cast<usize>(kMaxShaderDefineNameLen);
  }

  Copy(descr.name, name, descr.name_len);
  descr.name[descr.name_len] = '\0';
}

void SetValue(ShaderDefineDescr& descr, const schar* value, usize value_len) {
  COMET_ASSERT(value != nullptr, "rendering_shader_utils::SetValue",
               "value is null");

  descr.value_len = value_len;

  if (descr.value_len >= kMaxShaderDefineValueLen) {
    COMET_LOG_WARNING(LoggerType::Rendering, "rendering_shader_utils::SetValue",
                      "shader define value is too long and will be truncated",
                      "value_len", descr.value_len, "max_value_len",
                      kMaxShaderDefineValueLen);

    descr.value_len = static_cast<usize>(kMaxShaderDefineValueLen);
  }

  Copy(descr.value, value, descr.value_len);
  descr.value[descr.value_len] = '\0';
}

bool IsBufferBindingType(ShaderBindingType type) {
  return type == ShaderBindingType::UniformBuffer ||
         type == ShaderBindingType::StorageBuffer;
}

bool IsImageBindingType(ShaderBindingType type) {
  return type == ShaderBindingType::CombinedImageSampler ||
         type == ShaderBindingType::SampledImage ||
         type == ShaderBindingType::Sampler ||
         type == ShaderBindingType::StorageImage;
}
}  // namespace rendering
}  // namespace comet