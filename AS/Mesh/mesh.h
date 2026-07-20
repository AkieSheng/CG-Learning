#pragma once

#include <vector>
#include "vertex.h"
#include "pbr_material.h"
#include "matrix.h"

struct Mesh final {
  Mesh();
  ~Mesh();

  auto upload(std::vector<Vertex> const& vertices,
              std::vector<unsigned int> const& indices) -> void;
  auto draw() const -> void;
  auto destroy() -> void;

  auto setMaterial(PBRMaterial* mat) -> void { material = mat; }
  auto getMaterial() const -> PBRMaterial* { return material; }

  auto setModelMatrix(Matrix const& m) -> void { modelMatrix = m; }
  auto getModelMatrix() const -> Matrix const& { return modelMatrix; }

  auto setWorldCenter(Vec3f const& c) -> void { worldCenter = c; }
  auto getWorldCenter() const -> Vec3f const& { return worldCenter; }

  auto indexCount() const -> int { return numIndices; }

  unsigned int vao{};
  unsigned int vbo{};
  unsigned int ebo{};
  int numIndices{};
  PBRMaterial* material{};
  Matrix modelMatrix{};
  Vec3f worldCenter{};
};
