// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/opengl/handler/opengl_sampler_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/logger/logging.h"
#include "comet/render/driver/opengl/utils/opengl_sampler_utils.h"

namespace comet {
namespace render {
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
  COMET_ASSERT(handle, "SamplerHandler::GetOrGenerate",
               "sampler instance creation failed", "sampler_key", key);

  sampler->handle = handle;
  return handle;
}

void SamplerHandler::Destroy(SamplerHandle handle) {
  auto* sampler{samplers_.Get(handle)};

  if (!samplers_.Release(handle)) {
    return;
  }

  COMET_ASSERT(sampler->handle == handle, "SamplerHandler::Destroy",
               "sampler handle mismatch", "expected_handle", handle,
               "actual_handle", sampler->handle, "sampler_key", sampler->key);

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
  COMET_ASSERT(sampler != nullptr, "SamplerHandler::Get", "sampler not found",
               "sampler_handle", handle);
  return sampler;
}

void SamplerHandler::OnInitialize() {
  allocator_.Initialize();
  samplers_.Initialize();
}

void SamplerHandler::OnShutdown() {
  memory::PlatformAllocator tmp_allocator{kEngineMemoryTagRender};
  auto handles_to_destroy{Array<SamplerHandle>::WithCapacity(
      &tmp_allocator, samplers_.GetLiveCount())};

  samplers_.ForEachLive(
      [&handles_to_destroy](SamplerHandle handle, const Sampler*) {
        handles_to_destroy.PushLast(handle);
      });

  for (const auto handle : handles_to_destroy) {
    const auto ref_count{samplers_.GetRefCount(handle)};

    if (ref_count > 0) {
      COMET_LOG_WARNING(LoggerType::Rendering, "SamplerHandler::OnShutdown",
                        "forcing sampler destruction", "sampler_handle", handle,
                        "ref_count", ref_count, "sampler_key",
                        samplers_.Get(handle)->key);
    }

    auto* sampler{samplers_.Drain(handle)};

    if (sampler == nullptr) {
      continue;
    }

    COMET_ASSERT(sampler->handle == handle, "SamplerHandler::OnShutdown",
                 "sampler handle mismatch", "expected_handle", handle,
                 "actual_handle", sampler->handle, "sampler_key", sampler->key);

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
  COMET_ASSERT(sampler != nullptr, "SamplerHandler::Get", "sampler not found",
               "sampler_handle", handle);
  return sampler;
}

Sampler* SamplerHandler::GenerateSampler(const SamplerDescr& descr) {
  auto* sampler{allocator_.AllocateOneAndPopulate<Sampler>()};
  COMET_ASSERT(sampler != nullptr, "SamplerHandler::GenerateSampler",
               "sampler allocation failed");

  *sampler = {};

  glGenSamplers(1, &sampler->native_handle);
  COMET_ASSERT(sampler->native_handle != kInvalidGlNativeSamplerHandle,
               "SamplerHandler::GenerateSampler",
               "OpenGL sampler creation failed", "sampler_key",
               GenerateSamplerKey(descr));

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
}  // namespace render
}  // namespace comet