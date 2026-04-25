// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_MODEL_MODEL_RESOURCE_HANDLER_H_
#define COMET_COMET_RESOURCE_MODEL_MODEL_RESOURCE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/resource/handler/resource_handler.h"
#include "comet/resource/model/model_resource.h"
#include "comet/resource/resource.h"
#include "comet/resource/type/common.h"

namespace comet {
namespace resource {
class StaticModelResourceHandler
    : public ResourceHandler<StaticModelResourceTag, StaticModelResource> {
 public:
  using Base = ResourceHandler;

  explicit StaticModelResourceHandler(const ResourceHandlerDescr& descr);
  StaticModelResourceHandler(const StaticModelResourceHandler&) = delete;
  StaticModelResourceHandler(StaticModelResourceHandler&&) = delete;
  StaticModelResourceHandler& operator=(const StaticModelResourceHandler&) =
      delete;
  StaticModelResourceHandler& operator=(StaticModelResourceHandler&&) = delete;
  ~StaticModelResourceHandler() override = default;

  ResourceFile Pack(const StaticModelResource& resource,
                    CompressionMode compression_mode) override;
  void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
              StaticModelResource* resource) override;
};

class SkeletalModelResourceHandler
    : public ResourceHandler<SkeletalModelResourceTag, SkeletalModelResource> {
 public:
  using Base = ResourceHandler;

  explicit SkeletalModelResourceHandler(const ResourceHandlerDescr& descr);
  SkeletalModelResourceHandler(const SkeletalModelResourceHandler&) = delete;
  SkeletalModelResourceHandler(SkeletalModelResourceHandler&&) = delete;
  SkeletalModelResourceHandler& operator=(const SkeletalModelResourceHandler&) =
      delete;
  SkeletalModelResourceHandler& operator=(SkeletalModelResourceHandler&&) =
      delete;
  ~SkeletalModelResourceHandler() override = default;

  ResourceFile Pack(const SkeletalModelResource& resource,
                    CompressionMode compression_mode) override;
  void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
              SkeletalModelResource* resource) override;
};

class SkeletonResourceHandler
    : public ResourceHandler<SkeletonResourceTag, SkeletonResource> {
 public:
  using Base = ResourceHandler;

  explicit SkeletonResourceHandler(const ResourceHandlerDescr& descr);
  SkeletonResourceHandler(const SkeletonResourceHandler&) = delete;
  SkeletonResourceHandler(SkeletonResourceHandler&&) = delete;
  SkeletonResourceHandler& operator=(const SkeletonResourceHandler&) = delete;
  SkeletonResourceHandler& operator=(SkeletonResourceHandler&&) = delete;
  ~SkeletonResourceHandler() override = default;

  ResourceFile Pack(const SkeletonResource& resource,
                    CompressionMode compression_mode) override;
  void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
              SkeletonResource* resource) override;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_MODEL_MODEL_RESOURCE_HANDLER_H_