// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_EXPORTER_H_

#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/type/tstring.h"
#include "comet/resource/shader/shader_resource.h"
#include "editor/asset/exporter/asset_exporter.h"

namespace comet {
namespace editor {
namespace asset {
class ShaderExporter : public AssetExporter {
 public:
  ShaderExporter() = default;
  ShaderExporter(const ShaderExporter&) = delete;
  ShaderExporter(ShaderExporter&&) = delete;
  ShaderExporter& operator=(const ShaderExporter&) = delete;
  ShaderExporter& operator=(ShaderExporter&&) = delete;
  ~ShaderExporter() override = default;

  bool IsCompatible(CTStringView extension) const override;

 protected:
  void PopulateFiles(ResourceFilesContext& context) const override;

 private:
  struct ShaderContext {
    schar* file{nullptr};
    usize file_len{0};
    usize file_buffer_len{0};
    const tchar* asset_abs_path{nullptr};
  };

  static void DumpShaderModules(const nlohmann::json& shader_file,
                                resource::ShaderResource& shader);
  static void DumpDefines(const nlohmann::json& shader_file,
                          memory::Allocator* allocator,
                          resource::ShaderResource& shader);
  static void DumpBindings(const nlohmann::json& shader_file,
                           memory::Allocator* allocator,
                           resource::ShaderResource& shader);
  static void DumpPushConstants(const nlohmann::json& shader_file,
                                memory::Allocator* allocator,
                                resource::ShaderResource& shader);

  static void OnShaderSizeRequest(job::IOJobParamsHandle params_handle);
  static void OnShaderLoading(job::IOJobParamsHandle params_handle);
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_EXPORTER_H_