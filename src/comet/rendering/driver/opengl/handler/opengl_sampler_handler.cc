// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_sampler_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/logger.h"
#include "comet/rendering/driver/opengl/utils/opengl_sampler_utils.h"

namespace comet {
namespace rendering {
namespace gl {
SamplerHandler::SamplerHandler(const SamplerHandlerDescr& descr)
    : Handler{descr}, samplers_{&cache_allocator_, 256} {}

SamplerHandle SamplerHandler::GetOrGenerate(const SamplerDescr& descr) {
  const auto key{GenerateSamplerKey(descr)};

  if (const auto handle{samplers_.TryAcquire(key)}; handle) {
    return handle;
  }

  auto* sampler{GenerateSampler(descr)};
  sampler->key = key;

  const auto handle{samplers_.Create(key, sampler)};
  COMET_ASSERT(handle, "Failed to create instance for sampler!");

  sampler->handle = handle;
  return handle;
}

void SamplerHandler::Destroy(SamplerHandle handle) {
  auto* sampler{samplers_.Get(handle)};

  if (!samplers_.Release(handle)) {
    return;
  }

  COMET_ASSERT(sampler->handle == handle,
               "Sampler handle mismatch during destruction!");

  if (sampler->native_handle != kInvalidGlNativeSamplerHandle) {
    glDeleteSamplers(1, &sampler->native_handle);
    sampler->native_handle = kInvalidGlNativeSamplerHandle;
  }

  samplers_.Remove(handle);
  sampler->handle.Invalidate();
  allocator_.Deallocate(sampler);
}

const Sampler* SamplerHandler::Get(SamplerHandle handle) const {
  const auto* sampler{samplers_.TryGet(handle)};
  COMET_ASSERT(sampler != nullptr, "Requested sampler does not exist: ", handle,
               "!");
  return sampler;
}

void SamplerHandler::OnInitialize() {
  allocator_.Initialize();
  samplers_.Initialize();
}

void SamplerHandler::OnShutdown() {
  memory::PlatformAllocator tmp_allocator{memory::kEngineMemoryTagRendering};
  Array<SamplerHandle> handles_to_destroy{&tmp_allocator};

  samplers_.ForEachLive(
      [&handles_to_destroy](SamplerHandle handle, const Sampler*) {
        handles_to_destroy.PushBack(handle);
      });

  for (const auto handle : handles_to_destroy) {
    const auto ref_count{samplers_.GetRefCount(handle)};

    if (ref_count > 0) {
      COMET_LOG_RENDERING_WARNING("Forcing destruction of sampler handle ",
                                  handle, " with remaining ref count ",
                                  ref_count, ", key ",
                                  samplers_.Get(handle)->key, "!");
    }

    auto* sampler{samplers_.Drain(handle)};

    if (sampler == nullptr) {
      continue;
    }

    COMET_ASSERT(sampler->handle == handle,
                 "Sampler handle mismatch during shutdown destruction!");

    if (sampler->native_handle != kInvalidGlNativeSamplerHandle) {
      glDeleteSamplers(1, &sampler->native_handle);
      sampler->native_handle = kInvalidGlNativeSamplerHandle;
    }

    sampler->handle.Invalidate();
    allocator_.Deallocate(sampler);
  }

  samplers_.Destroy();
  allocator_.Destroy();
}

Sampler* SamplerHandler::Get(SamplerHandle handle) {
  auto* sampler{samplers_.TryGet(handle)};
  COMET_ASSERT(sampler != nullptr, "Requested sampler does not exist: ", handle,
               "!");
  return sampler;
}

Sampler* SamplerHandler::GenerateSampler(const SamplerDescr& descr) {
  auto* sampler{allocator_.AllocateOneAndPopulate<Sampler>()};
  COMET_ASSERT(sampler != nullptr, "Failed to allocate OpenGL sampler!");

  *sampler = {};

  glGenSamplers(1, &sampler->native_handle);
  COMET_ASSERT(sampler->native_handle != kInvalidGlNativeSamplerHandle,
               "Failed to create OpenGL sampler!");

  glSamplerParameteri(sampler->native_handle, GL_TEXTURE_WRAP_S,
                      static_cast<GLint>(descr.wrap_s));
  glSamplerParameteri(sampler->native_handle, GL_TEXTURE_WRAP_T,
                      static_cast<GLint>(descr.wrap_t));
  glSamplerParameteri(sampler->native_handle, GL_TEXTURE_WRAP_R,
                      static_cast<GLint>(descr.wrap_r));
  glSamplerParameteri(sampler->native_handle, GL_TEXTURE_MIN_FILTER,
                      static_cast<GLint>(descr.min_filter));
  glSamplerParameteri(sampler->native_handle, GL_TEXTURE_MAG_FILTER,
                      static_cast<GLint>(descr.mag_filter));
  glSamplerParameteri(sampler->native_handle, GL_TEXTURE_COMPARE_MODE,
                      static_cast<GLint>(descr.compare_mode));
  glSamplerParameteri(sampler->native_handle, GL_TEXTURE_COMPARE_FUNC,
                      static_cast<GLint>(descr.compare_func));

  if (descr.use_border_color) {
    const GLfloat color[4]{descr.border_color.x, descr.border_color.y,
                           descr.border_color.z, descr.border_color.w};
    glSamplerParameterfv(sampler->native_handle, GL_TEXTURE_BORDER_COLOR,
                         color);
  }

  return sampler;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet