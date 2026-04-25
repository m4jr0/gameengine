// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_SHADER_UTILS_SHADER_EXPORT_UTILS_H_
#define COMET_EDITOR_ASSET_EXPORTER_SHADER_UTILS_SHADER_EXPORT_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "nlohmann/json.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/type/pipeline.h"
#include "comet/rendering/type/shader.h"

namespace comet {
namespace editor {
namespace asset {
rendering::CullMode GetCullMode(std::string_view raw_cull_mode);

rendering::RasterizerDescr GetRasterizerDescr(
    const nlohmann::json& shader_file);

rendering::CompareOp GetCompareOp(std::string_view raw_compare_op);

rendering::DepthStencilDescr GetDepthStencilDescr(
    const nlohmann::json& shader_file);

rendering::PrimitiveTopology GetPrimitiveTopology(
    std::string_view raw_topology);

rendering::ShaderVertexLayout GetShaderVertexLayout(
    std::string_view raw_vertex_layout);

rendering::ShaderVariableType GetShaderVariableType(
    std::string_view raw_data_type);

rendering::ShaderBindingType GetShaderBindingType(
    std::string_view raw_binding_type);

rendering::ShaderBindingScope GetShaderBindingScope(
    std::string_view raw_binding_scope);

rendering::ShaderMemoryLayout GetShaderMemoryLayout(
    std::string_view raw_layout);

rendering::ShaderStageFlags GetShaderStageFlags(
    const nlohmann::json& raw_stages, memory::Allocator* allocator);

rendering::ShaderImageBindingSemantic GetShaderImageBindingSemantic(
    std::string_view raw_semantic);
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_SHADER_UTILS_SHADER_EXPORT_UTILS_H_