// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/handler/vulkan_sampler_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/allocator/allocator.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_sampler_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
SamplerHandler::SamplerHandler(const SamplerHandlerDescr& descr)
    : Handler{descr}, samplers_{&cache_allocator_, 256} {}

SamplerHandle SamplerHandler::GetOrGenerate(const SamplerDescr& descr) {
  const auto key{GenerateSamplerKey(descr)};

  if (const auto handle{samplers_.TryAcquire(key)}; handle) {
    return handle;
  }

  auto* sampler{GenerateSampler(key, descr)};

  const auto handle{samplers_.Create(key, sampler)};
  COMET_ASSERT(handle, "SamplerHandler::GetOrGenerate",
               "sampler instance creation failed", "sampler_key", key);

  sampler->handle = handle;
  return handle;
}

void SamplerHandler::Destroy(SamplerHandle handle) {
  auto* sampler{Get(handle)};

  COMET_ASSERT(sampler->handle == handle, "SamplerHandler::Destroy",
               "sampler handle mismatch", "expected_handle", handle,
               "actual_handle", sampler->handle, "sampler_key", sampler->key);

  if (!samplers_.Release(handle)) {
    return;
  }

  DestroySampler(sampler);
  samplers_.Remove(handle);
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
  memory::PlatformAllocator tmp_allocator{memory::kEngineMemoryTagRendering};
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

    DestroySampler(sampler);
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

Sampler* SamplerHandler::GenerateSampler(SamplerKey key,
                                         const SamplerDescr& descr) {
  auto* sampler{allocator_.AllocateOneAndPopulate<Sampler>()};
  sampler->handle = SamplerHandle::Invalid();
  sampler->key = key;

  VkSamplerCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  info.magFilter = descr.mag_filter;
  info.minFilter = descr.min_filter;
  info.mipmapMode = descr.mipmap_mode;
  info.addressModeU = descr.address_mode_u;
  info.addressModeV = descr.address_mode_v;
  info.addressModeW = descr.address_mode_w;
  info.mipLodBias = .0f;
  info.anisotropyEnable = VK_FALSE;
  info.maxAnisotropy = 1.0f;
  info.compareEnable = descr.compare_enable ? VK_TRUE : VK_FALSE;
  info.compareOp = descr.compare_op;
  info.minLod = descr.min_lod;
  info.maxLod = descr.max_lod;
  info.borderColor = descr.border_color;
  info.unnormalizedCoordinates =
      descr.unnormalized_coordinates ? VK_TRUE : VK_FALSE;

  COMET_CHECK_VK(vkCreateSampler(context_->GetDevice(), &info, nullptr,
                                 &sampler->native_handle),
                 "SamplerHandler::GenerateSampler", "sampler creation failed",
                 "sampler_key", key);

  return sampler;
}

void SamplerHandler::DestroySampler(Sampler* sampler) {
  COMET_ASSERT(sampler != nullptr, "SamplerHandler::DestroySampler",
               "sampler is null");

  if (sampler->native_handle != kInvalidVkNativeSamplerHandle) {
    vkDestroySampler(context_->GetDevice(), sampler->native_handle, nullptr);
    sampler->native_handle = kInvalidVkNativeSamplerHandle;
  }

  sampler->handle.Invalidate();
  allocator_.Deallocate(sampler);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet