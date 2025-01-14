// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "opengl_mesh_handler.h"

#include <utility>

#include "comet/profiler/profiler.h"

namespace comet {
namespace rendering {
namespace gl {
MeshHandler::MeshHandler(const MeshHandlerDescr& descr) : Handler{descr} {}

void MeshHandler::Shutdown() {
  std::vector<VertexArrayObjectHandle> vao_handles{};
  std::vector<VertexBufferObjectHandle> vbo_handles{};
  std::vector<ElementBufferObjectHandle> ebo_handles{};

  const auto total_handle_count{mesh_proxies_.size()};
  vao_handles.reserve(total_handle_count);
  vbo_handles.reserve(total_handle_count);
  ebo_handles.reserve(total_handle_count);

  for (auto& it : mesh_proxies_) {
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
    glDeleteVertexArrays(static_cast<s32>(vao_handles.size()),
                         vao_handles.data());
  }

  if (vbo_handles.size() > 0) {
    glDeleteBuffers(static_cast<s32>(vbo_handles.size()), vbo_handles.data());
  }

  if (ebo_handles.size() > 0) {
    glDeleteBuffers(static_cast<s32>(ebo_handles.size()), ebo_handles.data());
  }

  mesh_proxies_.clear();
  Handler::Shutdown();
}

MeshProxy* MeshHandler::Generate(const geometry::Mesh* mesh) {
  COMET_PROFILE("MeshHandler::Generate");
  MeshProxy proxy{};
  proxy.id = mesh->id;
  proxy.mesh = mesh;
  return Register(proxy);
}

MeshProxy* MeshHandler::Get(geometry::MeshId proxy_id) {
  auto* proxy{TryGet(proxy_id)};
  COMET_ASSERT(proxy != nullptr,
               "Requested mesh proxy does not exist: ", proxy_id, "!");
  return proxy;
}

MeshProxy* MeshHandler::Get(const geometry::Mesh* mesh) {
  return Get(mesh->id);
}

MeshProxy* MeshHandler::TryGet(geometry::MeshId proxy_id) {
  auto it{mesh_proxies_.find(proxy_id)};

  if (it != mesh_proxies_.end()) {
    return &it->second;
  }

  return nullptr;
}

MeshProxy* MeshHandler::TryGet(const geometry::Mesh* mesh) {
  return TryGet(mesh->id);
}

MeshProxy* MeshHandler::GetOrGenerate(const geometry::Mesh* mesh) {
  auto* proxy{TryGet(mesh)};

  if (proxy != nullptr) {
    return proxy;
  }

  return Generate(mesh);
}

void MeshHandler::Destroy(geometry::MeshId proxy_id) {
  Destroy(*Get(proxy_id));
}

void MeshHandler::Destroy(MeshProxy& mesh) { Destroy(mesh, false); }

void MeshHandler::Update(geometry::MeshId proxy_id) { Update(*Get(proxy_id)); }

void MeshHandler::Update(MeshProxy& proxy) {
  if (!proxy.mesh->is_dirty) {
    return;
  }

  Upload(proxy);
}

void MeshHandler::Bind(geometry::MeshId proxy_id) { Bind(Get(proxy_id)); }

void MeshHandler::Bind(const MeshProxy* proxy) {
  glBindVertexArray(proxy->vao_handle);
}

MeshProxy* MeshHandler::Register(MeshProxy& proxy) {
#ifdef COMET_DEBUG
  const auto proxy_id{proxy.id};
#endif  // COMET_DEBUG

  const auto insert_pair{mesh_proxies_.emplace(proxy.id, std::move(proxy))};
  MeshProxy* to_return{insert_pair.second ? &insert_pair.first->second
                                          : nullptr};

  if (to_return != nullptr) {
    Upload(*to_return);
  } else {
    COMET_ASSERT(false, "Could not insert mesh proxy #", proxy_id, "!");
  }

  return to_return;
}

void MeshHandler::Destroy(MeshProxy& proxy, bool is_destroying_handler) {
  COMET_PROFILE("MeshHandler::Destroy");
  if (!is_destroying_handler) {
    if (proxy.vao_handle != kInvalidVertexArrayObjectHandle) {
      glDeleteVertexArrays(1, &proxy.vao_handle);
    }

    if (proxy.vbo_handle != kInvalidVertexBufferObjectHandle) {
      glDeleteBuffers(1, &proxy.vbo_handle);
    }

    if (proxy.ebo_handle != kInvalidElementBufferObjectHandle) {
      glDeleteBuffers(1, &proxy.ebo_handle);
    }
  }

  proxy.vao_handle = kInvalidVertexArrayObjectHandle;
  proxy.vbo_handle = kInvalidVertexBufferObjectHandle;
  proxy.ebo_handle = kInvalidElementBufferObjectHandle;
  proxy.mesh = nullptr;

  if (!is_destroying_handler) {
    mesh_proxies_.erase(proxy.id);
  }

  proxy.id = geometry::kInvalidMeshId;
}

void MeshHandler::Upload(MeshProxy& proxy) const {
  if (proxy.vao_handle == kInvalidVertexArrayObjectHandle) {
    glGenVertexArrays(1, &proxy.vao_handle);
  }

  if (proxy.vbo_handle == kInvalidVertexBufferObjectHandle) {
    glGenBuffers(1, &proxy.vbo_handle);
  }

  if (proxy.ebo_handle == kInvalidElementBufferObjectHandle) {
    glGenBuffers(1, &proxy.ebo_handle);
  }

  auto usage{proxy.mesh->type == geometry::MeshType::Static ? GL_STATIC_DRAW
                                                            : GL_DYNAMIC_DRAW};

  glBindVertexArray(proxy.vao_handle);
  glBindBuffer(GL_ARRAY_BUFFER, proxy.vbo_handle);

  constexpr auto kVertexSize{sizeof(geometry::Vertex)};
  const auto* mesh{proxy.mesh};

  glBufferData(GL_ARRAY_BUFFER, mesh->vertices.GetSize() * kVertexSize,
               mesh->vertices.GetData(), usage);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, proxy.ebo_handle);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               mesh->indices.GetSize() * sizeof(geometry::Index),
               mesh->indices.GetData(), usage);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, kVertexSize,
                        reinterpret_cast<void*>(0));

  glEnableVertexAttribArray(1);

  glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, kVertexSize,
      reinterpret_cast<void*>(offsetof(geometry::Vertex, normal)));

  glEnableVertexAttribArray(2);

  glVertexAttribPointer(
      2, 4, GL_FLOAT, GL_FALSE, kVertexSize,
      reinterpret_cast<void*>(offsetof(geometry::Vertex, color)));

  glEnableVertexAttribArray(3);

  glVertexAttribPointer(
      3, 2, GL_FLOAT, GL_FALSE, kVertexSize,
      reinterpret_cast<void*>(offsetof(geometry::Vertex, uv)));

  glBindVertexArray(kInvalidVertexArrayObjectHandle);
  glBindBuffer(GL_ARRAY_BUFFER, kInvalidElementBufferObjectHandle);
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
