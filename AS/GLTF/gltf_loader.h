#ifndef _GLTF_LOADER_H_
#define _GLTF_LOADER_H_

#include <string>
#include <vector>
#include "mesh.h"
#include "vectors.h"
#include "matrix.h"

namespace tinygltf {
class Model;
class TinyGLTF;
struct Primitive;
}

// glTF 2.0 加载器（后端：tinygltf）
class GltfLoader {
public:
  GltfLoader();
  ~GltfLoader();

  bool load(const std::string &gltfPath);
  const std::vector<Mesh *> &getMeshes() const { return meshes; }

  // 场景 AABB
  void getBounds(Vec3f &bmin, Vec3f &bmax) const;
  void destroy();

  const std::string &getBasePath() const { return basePath; }

private:
  GltfLoader(const GltfLoader &);
  GltfLoader &operator=(const GltfLoader &);

  PBRMaterial *buildMaterial(int materialIndex);
  Mesh *buildPrimitiveMesh(const tinygltf::Primitive &prim, const Matrix &worldMatrix);
  void traverseNode(int nodeIndex, const Matrix &parentWorld);
  void computeTangents(std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);
  void expandBounds(const Vec3f &p);

  tinygltf::Model *model;
  tinygltf::TinyGLTF *loader;
  std::string basePath;
  std::vector<Mesh *> meshes;
  std::vector<PBRMaterial *> materials;
  bool boundsValid;
  Vec3f boundsMin;
  Vec3f boundsMax;
};

#endif
