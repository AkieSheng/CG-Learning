#pragma once

#include "mesh.h"
#include "vectors.h"
#include "matrix.h"
#include <string>
#include <vector>

namespace tinygltf
{
class Model;
class TinyGLTF;
struct Primitive;
}

struct GltfLoader final
{
  GltfLoader();
  ~GltfLoader();

  auto load(std::string const& gltfPath) -> bool;
  auto getMeshes() const -> std::vector<Mesh*> const& { return meshes; }
  auto getBounds(Vec3f& bmin, Vec3f& bmax) const -> void;
  auto destroy() -> void;
  auto getBasePath() const -> std::string const& { return basePath; }

  GltfLoader(GltfLoader const&) = delete;
  auto operator=(GltfLoader const&) -> GltfLoader& = delete;

  tinygltf::Model* model{};
  tinygltf::TinyGLTF* loader{};
  std::string basePath{};
  std::vector<Mesh*> meshes{};
  std::vector<PBRMaterial*> materials{};
  bool boundsValid{};
  Vec3f boundsMin{};
  Vec3f boundsMax{};

  auto buildMaterial(int materialIndex) -> PBRMaterial*;
  auto buildPrimitiveMesh(tinygltf::Primitive const& prim,
                          Matrix const& worldMatrix) -> Mesh*;
  auto traverseNode(int nodeIndex, Matrix const& parentWorld) -> void;
  auto computeTangents(std::vector<Vertex>& vertices,
                       std::vector<unsigned int> const& indices) -> void;
  auto expandBounds(Vec3f const& p) -> void;
};
