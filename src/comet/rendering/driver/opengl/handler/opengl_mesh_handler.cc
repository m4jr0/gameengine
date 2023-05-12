// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_mesh_handler.h"

namespace comet {
namespace rendering {
namespace gl {
MeshHandler::MeshHandler(const MeshHandlerDescr& descr) : Handler{descr} {}

void MeshHandler::Shutdown() {
  std::vector<VertexArrayObjectHandle> vao_handles{};
  std::vector<VertexBufferObjectHandle> vbo_handles{};
  std::vector<ElementBufferObjectHandle> ebo_handles{};
  vao_handles.reserve(meshes_.size());
  vbo_handles.reserve(meshes_.size());
  ebo_handles.reserve(meshes_.size());

  for (auto& it : meshes_) {
    auto& mesh{it.second};
    if (mesh.vao_handle != kInvalidVertexArrayObjectHandle) {
      vao_handles.push_back(mesh.vao_handle);
    }

    if (mesh.vbo_handle != kInvalidVertexBufferObjectHandle) {
      vao_handles.push_back(mesh.vbo_handle);
    }

    if (mesh.ebo_handle != kInvalidElementBufferObjectHandle) {
      vao_handles.push_back(mesh.ebo_handle);
    }

    Destroy(it.second, true);
  }

  if (vao_handles.size() > 0) {
    glDeleteVertexArrays(vao_handles.size(), vao_handles.data());
  }

  if (vbo_handles.size() > 0) {
    glDeleteBuffers(vbo_handles.size(), vbo_handles.data());
  }

  if (ebo_handles.size() > 0) {
    glDeleteBuffers(ebo_handles.size(), ebo_handles.data());
  }

  meshes_.clear();
  Handler::Shutdown();
}

Mesh* MeshHandler::Generate(const resource::MeshResource* resource) {
  Mesh mesh{};
  mesh.id = GenerateMeshId(resource);

  const auto vertex_count{resource->vertices.size()};
  mesh.vertices.resize(resource->vertices.size());

  for (uindex i{0}; i < vertex_count; ++i) {
    auto& vertex{mesh.vertices[i]};
    auto& vertex_res{resource->vertices[i]};

    vertex.position = vertex_res.position;
    vertex.normal = vertex_res.normal;

    vertex.uv.x = vertex_res.uv.x;
    vertex.uv.y = vertex_res.uv.y;

    vertex.color = {kColorWhite, 1.0f};
  }

  const auto index_count{resource->indices.size()};
  mesh.indices.resize(resource->indices.size());

  for (uindex i{0}; i < index_count; ++i) {
    mesh.indices[i] = resource->indices[i];
  }

#ifdef COMET_DEBUG
  const auto mesh_id{mesh.id};
#endif  // COMET_DEBUG

  const auto insert_pair{meshes_.emplace(mesh.id, std::move(mesh))};
  COMET_ASSERT(insert_pair.second,
               "Could not insert mesh: ", COMET_STRING_ID_LABEL(mesh_id), "!");
  auto& to_return{insert_pair.first->second};
  Upload(to_return);
  return &to_return;
}

Mesh* MeshHandler::Get(MeshId mesh_id) {
  auto* mesh{TryGet(mesh_id)};
  COMET_ASSERT(mesh != nullptr, "Requested mesh does not exist: ", mesh_id,
               "!");
  return mesh;
}

Mesh* MeshHandler::Get(const resource::MeshResource* resource) {
  return Get(GenerateMeshId(resource));
}

Mesh* MeshHandler::TryGet(MeshId mesh_id) {
  auto it{meshes_.find(mesh_id)};

  if (it == meshes_.end()) {
    return nullptr;
  }

  return &it->second;
}

Mesh* MeshHandler::TryGet(const resource::MeshResource* resource) {
  return TryGet(GenerateMeshId(resource));
}

Mesh* MeshHandler::GetOrGenerate(const resource::MeshResource* resource) {
  auto* mesh{TryGet(resource)};

  if (mesh != nullptr) {
    return mesh;
  }

  return Generate(resource);
}

void MeshHandler::Destroy(MeshId mesh_id) { Destroy(*Get(mesh_id)); }

void MeshHandler::Destroy(Mesh& mesh) { Destroy(mesh, false); }

MeshId MeshHandler::GenerateMeshId(
    const resource::MeshResource* resource) const {
  return static_cast<u64>(resource->internal_id) |
         static_cast<u64>(resource->resource_id) << 32;
}

void MeshHandler::Bind(MeshId mesh_id) { Bind(Get(mesh_id)); }

void MeshHandler::Bind(const Mesh* mesh) {
  glBindVertexArray(mesh->vao_handle);
}

void MeshHandler::Destroy(Mesh& mesh, bool is_destroying_handler) {
  if (!is_destroying_handler) {
    if (mesh.vao_handle != kInvalidVertexArrayObjectHandle) {
      glDeleteVertexArrays(1, &mesh.vao_handle);
    }

    if (mesh.vbo_handle != kInvalidVertexBufferObjectHandle) {
      glDeleteBuffers(1, &mesh.vbo_handle);
    }

    if (mesh.ebo_handle != kInvalidElementBufferObjectHandle) {
      glDeleteBuffers(1, &mesh.ebo_handle);
    }
  }

  mesh.vao_handle = kInvalidVertexArrayObjectHandle;
  mesh.vbo_handle = kInvalidVertexBufferObjectHandle;
  mesh.ebo_handle = kInvalidElementBufferObjectHandle;

  mesh.vertices.clear();
  mesh.indices.clear();

  if (!is_destroying_handler) {
    meshes_.erase(mesh.id);
  }

  mesh.id = kInvalidMeshId;
}

void MeshHandler::Upload(Mesh& mesh) const {
  glGenVertexArrays(1, &mesh.vao_handle);
  glGenBuffers(1, &mesh.vbo_handle);
  glGenBuffers(1, &mesh.ebo_handle);

  glBindVertexArray(mesh.vao_handle);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_handle);

  glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex),
               mesh.vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo_handle);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(Index),
               mesh.indices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void*>(0));

  glEnableVertexAttribArray(1);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void*>(offsetof(Vertex, normal)));

  glEnableVertexAttribArray(2);

  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void*>(offsetof(Vertex, color)));

  glEnableVertexAttribArray(3);

  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void*>(offsetof(Vertex, uv)));

  glBindVertexArray(kInvalidVertexArrayObjectHandle);
  glBindBuffer(GL_ARRAY_BUFFER, kInvalidElementBufferObjectHandle);
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
