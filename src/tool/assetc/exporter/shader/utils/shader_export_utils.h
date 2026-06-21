// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_SHADER_UTILS_SHADER_EXPORT_UTILS_H_
#define COMET_EDITOR_ASSET_EXPORTER_SHADER_UTILS_SHADER_EXPORT_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "nlohmann/json.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core.h"
#include "comet/data.h"

namespace comet {
namespace tool {
namespace assetc {
render::CullMode GetCullMode(std::string_view raw_cull_mode);

render::RasterizerDescr GetRasterizerDescr(const nlohmann::json& shader_file);

render::CompareOp GetCompareOp(std::string_view raw_compare_op);

render::DepthStencilDescr GetDepthStencilDescr(
    const nlohmann::json& shader_file);

render::PrimitiveTopology GetPrimitiveTopology(std::string_view raw_topology);

render::ShaderVertexLayout GetShaderVertexLayout(
    std::string_view raw_vertex_layout);

render::ShaderVariableType GetShaderVariableType(
    std::string_view raw_data_type);

render::ShaderBindingType GetShaderBindingType(
    std::string_view raw_binding_type);

render::ShaderBindingScope GetShaderBindingScope(
    std::string_view raw_binding_scope);

render::ShaderMemoryLayout GetShaderMemoryLayout(std::string_view raw_layout);

render::ShaderStageFlags GetShaderStageFlags(const nlohmann::json& raw_stages,
                                             memory::Allocator* allocator);

render::ShaderImageBindingSemantic GetShaderImageBindingSemantic(
    std::string_view raw_semantic);
}  // namespace assetc
}  // namespace tool
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_SHADER_UTILS_SHADER_EXPORT_UTILS_H_