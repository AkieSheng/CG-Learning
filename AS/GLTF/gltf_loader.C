#include "gltf_loader.h"
#include "texture.h"
#include "vertex.h"
#include "tiny_gltf.h"

#include <stdio.h>
#include <cmath>
#include <algorithm>

static std::string getDirectory(const std::string &path) {
  size_t pos = path.find_last_of("/\\");
  if (pos == std::string::npos) return "";
  return path.substr(0, pos + 1);
}

static Matrix matrixFromGltfColumnMajor(const std::vector<double> &m) {
  Matrix mat;
  if (m.size() < 16) {
    mat.SetToIdentity();
    return mat;
  }
  for (int col = 0; col < 4; col++) {
    for (int row = 0; row < 4; row++) {
      mat.Set(col, row, (float)m[(size_t)col * 4 + row]);
    }
  }
  return mat;
}

static Matrix matrixFromTRS(const std::vector<double> &t,
                            const std::vector<double> &r,
                            const std::vector<double> &s) {
  float tx = t.size() > 0 ? (float)t[0] : 0.0f;
  float ty = t.size() > 1 ? (float)t[1] : 0.0f;
  float tz = t.size() > 2 ? (float)t[2] : 0.0f;
  float sx = s.size() > 0 ? (float)s[0] : 1.0f;
  float sy = s.size() > 1 ? (float)s[1] : 1.0f;
  float sz = s.size() > 2 ? (float)s[2] : 1.0f;
  float qx = r.size() > 0 ? (float)r[0] : 0.0f;
  float qy = r.size() > 1 ? (float)r[1] : 0.0f;
  float qz = r.size() > 2 ? (float)r[2] : 0.0f;
  float qw = r.size() > 3 ? (float)r[3] : 1.0f;

  // glTF 四元数 → 矩阵（列向量）；Set(col, row) 写入 data[row][col]
  Matrix rot;
  rot.SetToIdentity();
  float x2 = qx + qx, y2 = qy + qy, z2 = qz + qz;
  float xx = qx * x2, xy = qx * y2, xz = qx * z2;
  float yy = qy * y2, yz = qy * z2, zz = qz * z2;
  float wx = qw * x2, wy = qw * y2, wz = qw * z2;
  rot.Set(0, 0, 1.0f - (yy + zz));
  rot.Set(0, 1, xy + wz);
  rot.Set(0, 2, xz - wy);
  rot.Set(1, 0, xy - wz);
  rot.Set(1, 1, 1.0f - (xx + zz));
  rot.Set(1, 2, yz + wx);
  rot.Set(2, 0, xz + wy);
  rot.Set(2, 1, yz - wx);
  rot.Set(2, 2, 1.0f - (xx + yy));

  Matrix scale = Matrix::MakeScale(Vec3f(sx, sy, sz));
  Matrix trans = Matrix::MakeTranslation(Vec3f(tx, ty, tz));
  return trans * rot * scale;
}

static const unsigned char *getAccessorPtr(const tinygltf::Model &model,
                                           const tinygltf::Accessor &acc) {
  if (acc.bufferView < 0 || acc.bufferView >= (int)model.bufferViews.size())
    return NULL;
  const tinygltf::BufferView &bv = model.bufferViews[acc.bufferView];
  if (bv.buffer < 0 || bv.buffer >= (int)model.buffers.size()) return NULL;
  const tinygltf::Buffer &buf = model.buffers[bv.buffer];
  return buf.data.data() + bv.byteOffset + acc.byteOffset;
}

static bool readFloatAttribute(const tinygltf::Model &model, int accessorIndex,
                               int type, int components,
                               std::vector<float> &out) {
  out.clear();
  if (accessorIndex < 0 || accessorIndex >= (int)model.accessors.size())
    return false;

  const tinygltf::Accessor &acc = model.accessors[accessorIndex];
  if (acc.type != type) return false;
  if (acc.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) return false;

  const unsigned char *base = getAccessorPtr(model, acc);
  if (!base) return false;

  int stride = acc.ByteStride(model.bufferViews[acc.bufferView]);
  if (stride <= 0) return false;

  out.resize(acc.count * (size_t)components);
  for (size_t i = 0; i < acc.count; i++) {
    const float *ptr = (const float *)(base + (size_t)i * (size_t)stride);
    for (int c = 0; c < components; c++)
      out[i * (size_t)components + c] = ptr[c];
  }
  return true;
}

static bool readIndices(const tinygltf::Model &model, int accessorIndex,
                        std::vector<unsigned int> &out) {
  out.clear();
  if (accessorIndex < 0 || accessorIndex >= (int)model.accessors.size())
    return false;

  const tinygltf::Accessor &acc = model.accessors[accessorIndex];
  const unsigned char *base = getAccessorPtr(model, acc);
  if (!base) return false;

  int stride = acc.ByteStride(model.bufferViews[acc.bufferView]);
  if (stride <= 0) {
    if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) stride = 4;
    else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) stride = 2;
    else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) stride = 1;
    else return false;
  }

  out.resize(acc.count);
  for (size_t i = 0; i < acc.count; i++) {
    const unsigned char *p = base + i * (size_t)stride;
    if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
      out[i] = *(const unsigned int *)p;
    } else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
      out[i] = *(const unsigned short *)p;
    } else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
      out[i] = p[0];
    } else {
      return false;
    }
  }
  return true;
}

static Texture *loadTextureFromModel(const tinygltf::Model &model,
                                     const std::string &basePath,
                                     int textureIndex, bool srgb) {
  if (textureIndex < 0 || textureIndex >= (int)model.textures.size()) return NULL;
  int imageIndex = model.textures[textureIndex].source;
  if (imageIndex < 0 || imageIndex >= (int)model.images.size()) return NULL;

  const std::string &uri = model.images[imageIndex].uri;
  if (uri.empty()) return NULL;

  std::string path = basePath + uri;
  Texture *tex = new Texture();
  if (!tex->loadFromFile(path, srgb)) {
    fprintf(stderr, "GltfLoader: failed to load texture %s\n", path.c_str());
    delete tex;
    return NULL;
  }
  return tex;
}

static void parseMaterialExtensions(const tinygltf::Material &gm,
                                    const tinygltf::Model &model,
                                    const std::string &basePath,
                                    PBRMaterial *mat) {
  tinygltf::ExtensionMap::const_iterator it;

  it = gm.extensions.find("KHR_materials_transmission");
  if (it != gm.extensions.end()) {
    const tinygltf::Value &ext = it->second;
    mat->hasTransmission = true;
    mat->ior = 1.5f;
    if (ext.Has("transmissionFactor"))
      mat->transmissionFactor = (float)ext.Get("transmissionFactor").GetNumberAsDouble();
    if (ext.Has("transmissionTexture")) {
      const tinygltf::Value &tex = ext.Get("transmissionTexture");
      if (tex.Has("index"))
        mat->transmissionTexture = loadTextureFromModel(model, basePath,
          tex.Get("index").GetNumberAsInt(), false);
    }
  }

  it = gm.extensions.find("KHR_materials_ior");
  if (it != gm.extensions.end()) {
    const tinygltf::Value &ext = it->second;
    if (ext.Has("ior")) {
      mat->ior = (float)ext.Get("ior").GetNumberAsDouble();
      mat->hasIor = true;
    }
  }

  it = gm.extensions.find("KHR_materials_volume");
  if (it != gm.extensions.end()) {
    const tinygltf::Value &ext = it->second;
    mat->hasVolume = true;
    if (ext.Has("thicknessFactor"))
      mat->volumeThickness = (float)ext.Get("thicknessFactor").GetNumberAsDouble();
    if (ext.Has("attenuationDistance"))
      mat->volumeAttenuationDistance = (float)ext.Get("attenuationDistance").GetNumberAsDouble();
    if (ext.Has("attenuationColor")) {
      const tinygltf::Value &c = ext.Get("attenuationColor");
      if (c.IsArray() && c.ArrayLen() >= 3) {
        mat->volumeAttenuationColor[0] = (float)c.Get(0).GetNumberAsDouble();
        mat->volumeAttenuationColor[1] = (float)c.Get(1).GetNumberAsDouble();
        mat->volumeAttenuationColor[2] = (float)c.Get(2).GetNumberAsDouble();
      }
    }
  }

  if (mat->hasTransmission && !mat->hasIor)
    mat->ior = 1.5f;

  it = gm.extensions.find("KHR_materials_clearcoat");
  if (it != gm.extensions.end()) {
    const tinygltf::Value &ext = it->second;
    mat->hasClearcoat = true;
    if (ext.Has("clearcoatFactor"))
      mat->clearcoatFactor = (float)ext.Get("clearcoatFactor").GetNumberAsDouble();
    if (ext.Has("clearcoatRoughnessFactor"))
      mat->clearcoatRoughnessFactor = (float)ext.Get("clearcoatRoughnessFactor").GetNumberAsDouble();
    if (ext.Has("clearcoatTexture")) {
      const tinygltf::Value &tex = ext.Get("clearcoatTexture");
      if (tex.Has("index"))
        mat->clearcoatTexture = loadTextureFromModel(model, basePath,
          tex.Get("index").GetNumberAsInt(), false);
    }
    if (ext.Has("clearcoatRoughnessTexture")) {
      const tinygltf::Value &tex = ext.Get("clearcoatRoughnessTexture");
      if (tex.Has("index"))
        mat->clearcoatRoughnessTexture = loadTextureFromModel(model, basePath,
          tex.Get("index").GetNumberAsInt(), false);
    }
    if (ext.Has("clearcoatNormalTexture")) {
      const tinygltf::Value &tex = ext.Get("clearcoatNormalTexture");
      if (tex.Has("index"))
        mat->clearcoatNormalTexture = loadTextureFromModel(model, basePath,
          tex.Get("index").GetNumberAsInt(), false);
      if (tex.Has("scale"))
        mat->clearcoatNormalScale = (float)tex.Get("scale").GetNumberAsDouble();
    }
  }
}

GltfLoader::GltfLoader()
  : model(NULL), loader(NULL), boundsValid(false) {}

GltfLoader::~GltfLoader() {
  destroy();
}

bool GltfLoader::load(const std::string &gltfPath) {
  destroy();

  model = new tinygltf::Model();
  loader = new tinygltf::TinyGLTF();

  basePath = getDirectory(gltfPath);
  std::string err, warn;

  bool ok = loader->LoadASCIIFromFile(model, &err, &warn, gltfPath);
  if (!warn.empty()) fprintf(stderr, "GltfLoader warn: %s\n", warn.c_str());
  if (!ok) {
    fprintf(stderr, "GltfLoader error: %s\n", err.c_str());
    destroy();
    return false;
  }

  meshes.clear();
  materials.clear();
  boundsValid = false;

  if (model->defaultScene >= 0 && model->defaultScene < (int)model->scenes.size()) {
    const tinygltf::Scene &scene = model->scenes[model->defaultScene];
    Matrix identity;
    identity.SetToIdentity();
    for (size_t i = 0; i < scene.nodes.size(); i++)
      traverseNode(scene.nodes[i], identity);
  } else {
    Matrix identity;
    identity.SetToIdentity();
    for (size_t i = 0; i < model->nodes.size(); i++) {
      bool isChild = false;
      for (size_t j = 0; j < model->nodes.size() && !isChild; j++) {
        const std::vector<int> &ch = model->nodes[j].children;
        for (size_t k = 0; k < ch.size(); k++) {
          if (ch[k] == (int)i) { isChild = true; break; }
        }
      }
      if (!isChild) traverseNode((int)i, identity);
    }
  }

  if (meshes.empty()) {
    fprintf(stderr, "GltfLoader: no meshes found in %s\n", gltfPath.c_str());
    destroy();
    return false;
  }

  printf("GltfLoader (tinygltf): %u mesh(es), %u material(s), scene %d\n",
         (unsigned)meshes.size(), (unsigned)materials.size(), model->defaultScene);
  return true;
}

PBRMaterial *GltfLoader::buildMaterial(int materialIndex) {
  if (materialIndex < 0) return NULL;
  if (materialIndex < (int)materials.size() && materials[materialIndex] != NULL)
    return materials[materialIndex];

  while ((int)materials.size() <= materialIndex) materials.push_back(NULL);
  if (materialIndex >= (int)model->materials.size()) return NULL;

  const tinygltf::Material &gm = model->materials[materialIndex];
  PBRMaterial *mat = new PBRMaterial();
  mat->name = gm.name;
  // glTF 2.0：metallicFactor / roughnessFactor 默认为 1.0
  mat->metallicFactor = (float)gm.pbrMetallicRoughness.metallicFactor;
  mat->roughnessFactor = (float)gm.pbrMetallicRoughness.roughnessFactor;
  mat->normalScale = (float)gm.normalTexture.scale;
  mat->occlusionStrength = (float)gm.occlusionTexture.strength;
  mat->alphaCutoff = (float)gm.alphaCutoff;
  mat->doubleSided = gm.doubleSided;

  const std::vector<double> &bcf = gm.pbrMetallicRoughness.baseColorFactor;
  for (int i = 0; i < 4; i++)
    mat->baseColorFactor[i] = i < (int)bcf.size() ? (float)bcf[i] : 1.0f;
  for (int i = 0; i < 3; i++)
    mat->emissiveFactor[i] = i < (int)gm.emissiveFactor.size() ? (float)gm.emissiveFactor[i] : 0.0f;

  if (gm.alphaMode == "MASK") mat->alphaMode = ALPHA_MASK;
  else if (gm.alphaMode == "BLEND") mat->alphaMode = ALPHA_BLEND;
  else mat->alphaMode = ALPHA_OPAQUE;

  mat->baseColorTexture = loadTextureFromModel(*model, basePath,
    gm.pbrMetallicRoughness.baseColorTexture.index, true);
  mat->metallicRoughnessTexture = loadTextureFromModel(*model, basePath,
    gm.pbrMetallicRoughness.metallicRoughnessTexture.index, false);
  mat->normalTexture = loadTextureFromModel(*model, basePath,
    gm.normalTexture.index, false);
  mat->occlusionTexture = loadTextureFromModel(*model, basePath,
    gm.occlusionTexture.index, false);
  mat->emissiveTexture = loadTextureFromModel(*model, basePath,
    gm.emissiveTexture.index, true);

  parseMaterialExtensions(gm, *model, basePath, mat);
  materials[materialIndex] = mat;
  return mat;
}

void GltfLoader::computeTangents(std::vector<Vertex> &vertices,
                                 const std::vector<unsigned int> &indices) {
  std::vector<Vec3f> tan1(vertices.size());
  std::vector<Vec3f> tan2(vertices.size());
  for (size_t i = 0; i < tan1.size(); i++) {
    tan1[i].Set(0, 0, 0);
    tan2[i].Set(0, 0, 0);
  }

  for (size_t i = 0; i + 2 < indices.size(); i += 3) {
    unsigned int i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];
    if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size())
      continue;

    const Vec3f &p0 = vertices[i0].position;
    const Vec3f &p1 = vertices[i1].position;
    const Vec3f &p2 = vertices[i2].position;
    float u0 = vertices[i0].texCoord0.x(), v0 = vertices[i0].texCoord0.y();
    float u1 = vertices[i1].texCoord0.x(), v1 = vertices[i1].texCoord0.y();
    float u2 = vertices[i2].texCoord0.x(), v2 = vertices[i2].texCoord0.y();

    float x1 = p1.x() - p0.x(), x2 = p2.x() - p0.x();
    float y1 = p1.y() - p0.y(), y2 = p2.y() - p0.y();
    float z1 = p1.z() - p0.z(), z2 = p2.z() - p0.z();
    float s1 = u1 - u0, s2 = u2 - u0;
    float t1 = v1 - v0, t2 = v2 - v0;
    float denom = s1 * t2 - s2 * t1;
    if (fabsf(denom) < 1e-8f) continue;
    float r = 1.0f / denom;

    Vec3f sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
    Vec3f tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);
    tan1[i0] += sdir; tan1[i1] += sdir; tan1[i2] += sdir;
    tan2[i0] += tdir; tan2[i1] += tdir; tan2[i2] += tdir;
  }

  for (size_t i = 0; i < vertices.size(); i++) {
    const Vec3f &n = vertices[i].normal;
    Vec3f t = tan1[i];
    Vec3f tangent = t - n * n.Dot3(t);
    tangent.Normalize();
    Vec3f bitangent;
    Vec3f::Cross3(bitangent, n, tangent);
    float w = (bitangent.Dot3(tan2[i]) < 0.0f) ? -1.0f : 1.0f;
    vertices[i].tangent.Set(tangent.x(), tangent.y(), tangent.z(), w);
  }
}

void GltfLoader::expandBounds(const Vec3f &p) {
  if (!boundsValid) {
    boundsMin = p;
    boundsMax = p;
    boundsValid = true;
  } else {
    if (p.x() < boundsMin.x()) boundsMin.Set(p.x(), boundsMin.y(), boundsMin.z());
    if (p.y() < boundsMin.y()) boundsMin.Set(boundsMin.x(), p.y(), boundsMin.z());
    if (p.z() < boundsMin.z()) boundsMin.Set(boundsMin.x(), boundsMin.y(), p.z());
    if (p.x() > boundsMax.x()) boundsMax.Set(p.x(), boundsMax.y(), boundsMax.z());
    if (p.y() > boundsMax.y()) boundsMax.Set(boundsMax.x(), p.y(), boundsMax.z());
    if (p.z() > boundsMax.z()) boundsMax.Set(boundsMax.x(), boundsMax.y(), p.z());
  }
}

Mesh *GltfLoader::buildPrimitiveMesh(const tinygltf::Primitive &prim,
                                     const Matrix &worldMatrix) {
  std::map<std::string, int>::const_iterator itPos = prim.attributes.find("POSITION");
  if (itPos == prim.attributes.end()) return NULL;

  int posAcc = itPos->second;
  if (posAcc < 0 || posAcc >= (int)model->accessors.size()) return NULL;
  size_t vertexCount = model->accessors[posAcc].count;

  std::vector<float> positions, normals, tangents, uvs;
  if (!readFloatAttribute(*model, posAcc, TINYGLTF_TYPE_VEC3, 3, positions))
    return NULL;

  std::map<std::string, int>::const_iterator itN = prim.attributes.find("NORMAL");
  bool hasNormal = (itN != prim.attributes.end()) &&
    readFloatAttribute(*model, itN->second, TINYGLTF_TYPE_VEC3, 3, normals) &&
    normals.size() == vertexCount * 3;

  std::map<std::string, int>::const_iterator itT = prim.attributes.find("TANGENT");
  bool hasTangent = (itT != prim.attributes.end()) &&
    readFloatAttribute(*model, itT->second, TINYGLTF_TYPE_VEC4, 4, tangents) &&
    tangents.size() == vertexCount * 4;

  std::map<std::string, int>::const_iterator itUv = prim.attributes.find("TEXCOORD_0");
  bool hasUv = (itUv != prim.attributes.end()) &&
    readFloatAttribute(*model, itUv->second, TINYGLTF_TYPE_VEC2, 2, uvs) &&
    uvs.size() == vertexCount * 2;

  std::vector<unsigned int> indices;
  if (prim.indices >= 0) {
    if (!readIndices(*model, prim.indices, indices)) return NULL;
  } else {
    indices.resize(vertexCount);
    for (size_t i = 0; i < vertexCount; i++) indices[i] = (unsigned int)i;
  }

  std::vector<Vertex> vertices(vertexCount);
  Vec3f centerSum(0.0f, 0.0f, 0.0f);
  for (size_t i = 0; i < vertexCount; i++) {
    Vertex &v = vertices[i];
    v.position.Set(positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2]);
    if (hasNormal)
      v.normal.Set(normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2]);
    else
      v.normal.Set(0, 1, 0);
    if (hasTangent)
      v.tangent.Set(tangents[i * 4 + 0], tangents[i * 4 + 1],
                    tangents[i * 4 + 2], tangents[i * 4 + 3]);
    else
      v.tangent.Set(1, 0, 0, 1);
    if (hasUv)
      v.texCoord0.Set(uvs[i * 2 + 0], uvs[i * 2 + 1]);
    else
      v.texCoord0.Set(0, 0);

    Vec3f worldPos = v.position;
    worldMatrix.Transform(worldPos);
    expandBounds(worldPos);
    centerSum += worldPos;
  }

  if (!hasTangent && hasUv)
    computeTangents(vertices, indices);

  Mesh *mesh = new Mesh();
  mesh->upload(vertices, indices);
  mesh->setModelMatrix(worldMatrix);
  if (vertexCount > 0) {
    centerSum /= (float)vertexCount;
    mesh->setWorldCenter(centerSum);
  }
  mesh->setMaterial(buildMaterial(prim.material));
  return mesh;
}

void GltfLoader::traverseNode(int nodeIndex, const Matrix &parentWorld) {
  if (!model || nodeIndex < 0 || nodeIndex >= (int)model->nodes.size()) return;

  const tinygltf::Node &node = model->nodes[nodeIndex];
  Matrix local;
  if (node.matrix.size() == 16) {
    local = matrixFromGltfColumnMajor(node.matrix);
  } else if (!node.translation.empty() || !node.rotation.empty() || !node.scale.empty()) {
    local = matrixFromTRS(node.translation, node.rotation, node.scale);
  } else {
    local.SetToIdentity();
  }

  Matrix world = parentWorld * local;

  if (node.mesh >= 0 && node.mesh < (int)model->meshes.size()) {
    const tinygltf::Mesh &gm = model->meshes[node.mesh];
    for (size_t i = 0; i < gm.primitives.size(); i++) {
      Mesh *m = buildPrimitiveMesh(gm.primitives[i], world);
      if (m) meshes.push_back(m);
    }
  }

  for (size_t i = 0; i < node.children.size(); i++)
    traverseNode(node.children[i], world);
}

void GltfLoader::getBounds(Vec3f &bmin, Vec3f &bmax) const {
  if (boundsValid) {
    bmin = boundsMin;
    bmax = boundsMax;
  } else {
    bmin.Set(-1, -1, -1);
    bmax.Set(1, 1, 1);
  }
}

void GltfLoader::destroy() {
  for (size_t i = 0; i < meshes.size(); i++) delete meshes[i];
  meshes.clear();

  for (size_t i = 0; i < materials.size(); i++) delete materials[i];
  materials.clear();

  delete model;
  model = NULL;
  delete loader;
  loader = NULL;

  basePath.clear();
  boundsValid = false;
}
