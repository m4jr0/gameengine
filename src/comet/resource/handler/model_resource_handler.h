// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_HANDLER_MODEL_RESOURCE_HANDLER_H_
#define COMET_COMET_RESOURCE_HANDLER_MODEL_RESOURCE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/resource/handler/resource_handler.h"
#include "comet/resource/model_resource.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
class StaticModelResourceHandler : public ResourceHandler<StaticModelResource> {
 public:
  StaticModelResourceHandler(const ResourceHandlerDescr& descr);
  StaticModelResourceHandler(const StaticModelResourceHandler&) = delete;
  StaticModelResourceHandler(StaticModelResourceHandler&&) = delete;
  StaticModelResourceHandler& operator=(const StaticModelResourceHandler&) =
      delete;
  StaticModelResourceHandler& operator=(StaticModelResourceHandler&&) = delete;
  virtual ~StaticModelResourceHandler() = default;

  ResourceFile Pack(const StaticModelResource& resource,
                    CompressionMode compression_mode) override;
  void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
              StaticModelResource* resource) override;
};

class SkeletalModelResourceHandler
    : public ResourceHandler<SkeletalModelResource> {
 public:
  SkeletalModelResourceHandler(const ResourceHandlerDescr& descr);
  SkeletalModelResourceHandler(const SkeletalModelResourceHandler&) = delete;
  SkeletalModelResourceHandler(SkeletalModelResourceHandler&&) = delete;
  SkeletalModelResourceHandler& operator=(const SkeletalModelResourceHandler&) =
      delete;
  SkeletalModelResourceHandler& operator=(SkeletalModelResourceHandler&&) =
      delete;
  virtual ~SkeletalModelResourceHandler() = default;

  ResourceFile Pack(const SkeletalModelResource& resource,
                    CompressionMode compression_mode) override;
  void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
              SkeletalModelResource* resource) override;
};

class SkeletonResourceHandler : public ResourceHandler<SkeletonResource> {
 public:
  SkeletonResourceHandler(const ResourceHandlerDescr& descr);
  SkeletonResourceHandler(const SkeletonResourceHandler&) = delete;
  SkeletonResourceHandler(SkeletonResourceHandler&&) = delete;
  SkeletonResourceHandler& operator=(const SkeletonResourceHandler&) = delete;
  SkeletonResourceHandler& operator=(SkeletonResourceHandler&&) = delete;
  virtual ~SkeletonResourceHandler() = default;

  ResourceFile Pack(const SkeletonResource& resource,
                    CompressionMode compression_mode) override;
  void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
              SkeletonResource* resource) override;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_HANDLER_MODEL_RESOURCE_HANDLER_H_
