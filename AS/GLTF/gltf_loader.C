#include "gltf_loader.h"
#include "texture.h"
#include "vertex.h"
#include "tiny_gltf.h"

#include <stdio.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>

static std::string getDirectory(std::string const& path) {
  size_t pos = path.find_last_of("/\\");
  if (pos == std::string::npos) return "";
  return path.substr(0, pos + 1);
}

static Matrix matrixFromGltfColumnMajor(std::vector<double> const& m) {
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

static Matrix matrixFromTRS(std::vector<double> const& t,
                            std::vector<double> const& r,
                            std::vector<double> const& s) {
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

static unsigned char const* getAccessorPtr(const tinygltf::Model &model,
                                           const tinygltf::Accessor &acc) {
  if (acc.bufferView < 0 || acc.bufferView >= static_cast<int>(model.bufferViews.size()))
    return nullptr;
  const tinygltf::BufferView &bv = model.bufferViews[acc.bufferView];
  if (bv.buffer < 0 || bv.buffer >= static_cast<int>(model.buffers.size())) return nullptr;
  const tinygltf::Buffer &buf = model.buffers[bv.buffer];
  return buf.data.data() + bv.byteOffset + acc.byteOffset;
}

static bool readFloatAttribute(const tinygltf::Model &model, int accessorIndex,
                               int type, int components,
                               std::vector<float>& out) {
  out.clear();
  if (accessorIndex < 0 ||
      accessorIndex >= static_cast<int>(model.accessors.size())) {
    return false;
  }

  const tinygltf::Accessor &acc = model.accessors[accessorIndex];
  if (acc.type != type) return false;
  if (acc.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) return false;

  unsigned char const* base = getAccessorPtr(model, acc);
  if (!base) return false;

  int stride = acc.ByteStride(model.bufferViews[acc.bufferView]);
  if (stride <= 0) return false;

  out.resize(acc.count * (size_t)components);
  for (size_t i = 0; i < acc.count; i++) {
    float const* ptr = (float const*)(base + (size_t)i * (size_t)stride);
    for (int c = 0; c < components; c++)
      out[i * (size_t)components + c] = ptr[c];
  }
  return true;
}

static bool readIndices(const tinygltf::Model &model, int accessorIndex,
                        std::vector<unsigned int>& out) {
  out.clear();
  if (accessorIndex < 0 ||
      accessorIndex >= static_cast<int>(model.accessors.size())) {
    return false;
  }

  const tinygltf::Accessor &acc = model.accessors[accessorIndex];
  unsigned char const* base = getAccessorPtr(model, acc);
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
    unsigned char const* p = base + i * (size_t)stride;
    if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
      out[i] = *reinterpret_cast<unsigned int const*>(p);
    } else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
      out[i] = *reinterpret_cast<unsigned short const*>(p);
    } else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
      out[i] = p[0];
    } else {
      return false;
    }
  }
  return true;
}

static Texture *loadTextureFromModel(const tinygltf::Model &model,
                                     std::string const& basePath,
                                     int textureIndex, bool srgb) {
  if (textureIndex < 0 || textureIndex >= static_cast<int>(model.textures.size())) return nullptr;
  int imageIndex = model.textures[textureIndex].source;
  if (imageIndex < 0 || imageIndex >= static_cast<int>(model.images.size())) return nullptr;

  std::string const& uri = model.images[imageIndex].uri;
  if (uri.empty()) return nullptr;

  std::string path = basePath + uri;
  Texture *tex = new Texture();
  if (!tex->loadFromFile(path, srgb)) {
    std::fprintf(stderr, "GltfLoader: failed to load texture %s\n", path.c_str());
    delete tex;
    return nullptr;
  }
  return tex;
}

static bool resolveTexturePath(const tinygltf::Model &model,
                               std::string const& basePath,
                               int textureIndex, std::string &outPath) {
  if (textureIndex < 0 || textureIndex >= static_cast<int>(model.textures.size())) return false;
  int imageIndex = model.textures[textureIndex].source;
  if (imageIndex < 0 || imageIndex >= static_cast<int>(model.images.size())) return false;
  std::string const& uri = model.images[imageIndex].uri;
  if (uri.empty()) return false;
  outPath = basePath + uri;
  return true;
}

static float clamp01(float v) {
  return std::max(0.0f, std::min(1.0f, v));
}

static float srgbToLinear(float c) {
  return (c <= 0.04045f) ? (c / 12.92f) : std::pow((c + 0.055f) / 1.055f, 2.4f);
}

static float linearToSrgb(float c) {
  c = std::max(0.0f, c);
  return (c <= 0.0031308f) ? (c * 12.92f) : (1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f);
}

static float const* srgbToLinearLut() {
  static float lut[256];
  static bool ready = false;
  if (!ready) {
    for (int i = 0; i < 256; i++)
      lut[i] = srgbToLinear((float)i / 255.0f);
    ready = true;
  }
  return lut;
}

static unsigned char linearToSrgbByte(float c) {
  static unsigned char lut[1025];
  static bool ready = false;
  if (!ready) {
    for (int i = 0; i <= 1024; i++)
      lut[i] = (unsigned char)(linearToSrgb((float)i / 1024.0f) * 255.0f + 0.5f);
    ready = true;
  }
  c = clamp01(c);
  int idx = (int)(c * 1024.0f + 0.5f);
  if (idx > 1024) idx = 1024;
  return lut[idx];
}

static float perceivedBrightness(float r, float g, float b) {
  return std::sqrt(0.299f * r * r + 0.587f * g * g + 0.114f * b * b);
}

static void convertSpecGlossPixel(float diffuseR, float diffuseG, float diffuseB, float diffuseA,
                                  float specularR, float specularG, float specularB,
                                  float glossiness,
                                  float &outBaseR, float &outBaseG, float &outBaseB, float &outBaseA,
                                  float &outMetallic, float &outRoughness) {
  const float dielectricSpecular = 0.04f;
  const float epsilon = 1e-6f;

  float oneMinusSpecularStrength =
      1.0f - std::max(specularR, std::max(specularG, specularB));
  float diffuseBrightness = perceivedBrightness(diffuseR, diffuseG, diffuseB);
  float specularBrightness = perceivedBrightness(specularR, specularG, specularB);

  float metallic = 0.0f;
  if (specularBrightness >= dielectricSpecular) {
    float a = dielectricSpecular;
    float b = diffuseBrightness * oneMinusSpecularStrength / (1.0f - dielectricSpecular)
              + specularBrightness - 2.0f * dielectricSpecular;
    float c = dielectricSpecular - specularBrightness;
    float D = std::max(b * b - 4.0f * a * c, 0.0f);
    metallic = clamp01((-b + std::sqrt(D)) / (2.0f * a));
  }

  float invOneMinusMetal = 1.0f / std::max(1.0f - metallic, epsilon);
  float invMetal = 1.0f / std::max(metallic, epsilon);
  float scaleDiff = oneMinusSpecularStrength / (1.0f - dielectricSpecular) * invOneMinusMetal;

  float baseFromDiffR = diffuseR * scaleDiff;
  float baseFromDiffG = diffuseG * scaleDiff;
  float baseFromDiffB = diffuseB * scaleDiff;

  float baseFromSpecR = (specularR - dielectricSpecular * (1.0f - metallic)) * invMetal;
  float baseFromSpecG = (specularG - dielectricSpecular * (1.0f - metallic)) * invMetal;
  float baseFromSpecB = (specularB - dielectricSpecular * (1.0f - metallic)) * invMetal;

  float w = metallic * metallic;
  outBaseR = clamp01(baseFromDiffR * (1.0f - w) + baseFromSpecR * w);
  outBaseG = clamp01(baseFromDiffG * (1.0f - w) + baseFromSpecG * w);
  outBaseB = clamp01(baseFromDiffB * (1.0f - w) + baseFromSpecB * w);
  outBaseA = clamp01(diffuseA);
  outMetallic = metallic;
  outRoughness = clamp01(1.0f - glossiness);
}

static void sampleRGBANearestBytes(std::vector<unsigned char> &rgba, int w, int h,
                                   int x, int y, unsigned char out[4]) {
  x = std::max(0, std::min(w - 1, x));
  y = std::max(0, std::min(h - 1, y));
  size_t i = ((size_t)y * (size_t)w + (size_t)x) * 4;
  out[0] = rgba[i + 0];
  out[1] = rgba[i + 1];
  out[2] = rgba[i + 2];
  out[3] = rgba[i + 3];
}

static bool applySpecularGlossinessConversion(const tinygltf::Material &gm,
                                              const tinygltf::Model &model,
                                              std::string const& basePath,
                                              PBRMaterial *mat) {
  tinygltf::ExtensionMap::const_iterator it =
      gm.extensions.find("KHR_materials_pbrSpecularGlossiness");
  if (it == gm.extensions.end()) return false;

  const tinygltf::Value &ext = it->second;

  float diffuseFactor[4] = {1, 1, 1, 1};
  float specularFactor[3] = {1, 1, 1};
  float glossinessFactor = 1.0f;
  int diffuseTexIndex = -1;
  int specGlossTexIndex = -1;

  if (ext.Has("diffuseFactor") && ext.Get("diffuseFactor").IsArray()) {
    const tinygltf::Value &arr = ext.Get("diffuseFactor");
    for (int i = 0; i < 4 && i < static_cast<int>(arr.ArrayLen()); i++)
      diffuseFactor[i] = static_cast<float>(arr.Get(i).GetNumberAsDouble());
  }
  if (ext.Has("specularFactor") && ext.Get("specularFactor").IsArray()) {
    const tinygltf::Value &arr = ext.Get("specularFactor");
    for (int i = 0; i < 3 && i < static_cast<int>(arr.ArrayLen()); i++)
      specularFactor[i] = static_cast<float>(arr.Get(i).GetNumberAsDouble());
  }
  if (ext.Has("glossinessFactor"))
    glossinessFactor = static_cast<float>(ext.Get("glossinessFactor").GetNumberAsDouble());
  if (ext.Has("diffuseTexture") && ext.Get("diffuseTexture").Has("index"))
    diffuseTexIndex = ext.Get("diffuseTexture").Get("index").GetNumberAsInt();
  if (ext.Has("specularGlossinessTexture") &&
      ext.Get("specularGlossinessTexture").Has("index"))
    specGlossTexIndex =
        ext.Get("specularGlossinessTexture").Get("index").GetNumberAsInt();

  std::vector<unsigned char> diffusePixels, specGlossPixels;
  int diffW = 0, diffH = 0, sgW = 0, sgH = 0;
  bool hasDiffTex = false, hasSgTex = false;

  if (diffuseTexIndex >= 0) {
    std::string path;
    if (resolveTexturePath(model, basePath, diffuseTexIndex, path) &&
        Texture::loadPixelsRGBA(path, diffusePixels, diffW, diffH))
      hasDiffTex = true;
    else
      std::fprintf(stderr, "GltfLoader: SpecGloss diffuse texture failed\n");
  }
  if (specGlossTexIndex >= 0) {
    std::string path;
    if (resolveTexturePath(model, basePath, specGlossTexIndex, path) &&
        Texture::loadPixelsRGBA(path, specGlossPixels, sgW, sgH))
      hasSgTex = true;
    else
      std::fprintf(stderr, "GltfLoader: SpecGloss specularGlossiness texture failed\n");
  }

  delete mat->baseColorTexture;
  delete mat->metallicRoughnessTexture;
  mat->baseColorTexture = nullptr;
  mat->metallicRoughnessTexture = nullptr;

  if (!hasDiffTex && !hasSgTex) {
    float baseR, baseG, baseB, baseA, metallic, roughness;
    convertSpecGlossPixel(
        diffuseFactor[0], diffuseFactor[1], diffuseFactor[2], diffuseFactor[3],
        specularFactor[0], specularFactor[1], specularFactor[2], glossinessFactor,
        baseR, baseG, baseB, baseA, metallic, roughness);
    mat->baseColorFactor[0] = baseR;
    mat->baseColorFactor[1] = baseG;
    mat->baseColorFactor[2] = baseB;
    mat->baseColorFactor[3] = baseA;
    mat->metallicFactor = metallic;
    mat->roughnessFactor = roughness;
    std::fprintf(stderr, "GltfLoader: converted SpecGloss→MR (factors only) for '%s'\n",
           mat->name.c_str());
    return true;
  }

  int outW = std::max(hasDiffTex ? diffW : 1, hasSgTex ? sgW : 1);
  int outH = std::max(hasDiffTex ? diffH : 1, hasSgTex ? sgH : 1);
  std::vector<unsigned char> baseColorOut((size_t)outW * (size_t)outH * 4);
  std::vector<unsigned char> mrOut((size_t)outW * (size_t)outH * 4);

  float const* toLinear = srgbToLinearLut();
  for (int y = 0; y < outH; y++) {
    for (int x = 0; x < outW; x++) {
      float diffLin[4] = {
          diffuseFactor[0], diffuseFactor[1], diffuseFactor[2], diffuseFactor[3]};
      float specLin[3] = {
          specularFactor[0], specularFactor[1], specularFactor[2]};
      float gloss = glossinessFactor;

      if (hasDiffTex) {
        unsigned char s[4];
        int sx = (diffW == outW) ? x : (x * diffW / outW);
        int sy = (diffH == outH) ? y : (y * diffH / outH);
        sampleRGBANearestBytes(diffusePixels, diffW, diffH, sx, sy, s);
        diffLin[0] *= toLinear[s[0]];
        diffLin[1] *= toLinear[s[1]];
        diffLin[2] *= toLinear[s[2]];
        diffLin[3] *= (float)s[3] / 255.0f;
      }
      if (hasSgTex) {
        unsigned char s[4];
        int sx = (sgW == outW) ? x : (x * sgW / outW);
        int sy = (sgH == outH) ? y : (y * sgH / outH);
        sampleRGBANearestBytes(specGlossPixels, sgW, sgH, sx, sy, s);
        specLin[0] *= toLinear[s[0]];
        specLin[1] *= toLinear[s[1]];
        specLin[2] *= toLinear[s[2]];
        gloss *= (float)s[3] / 255.0f;
      }

      float baseR, baseG, baseB, baseA, metallic, roughness;
      convertSpecGlossPixel(
          diffLin[0], diffLin[1], diffLin[2], diffLin[3],
          specLin[0], specLin[1], specLin[2], gloss,
          baseR, baseG, baseB, baseA, metallic, roughness);

      size_t i = ((size_t)y * (size_t)outW + (size_t)x) * 4;
      baseColorOut[i + 0] = linearToSrgbByte(baseR);
      baseColorOut[i + 1] = linearToSrgbByte(baseG);
      baseColorOut[i + 2] = linearToSrgbByte(baseB);
      baseColorOut[i + 3] = (unsigned char)(clamp01(baseA) * 255.0f + 0.5f);
      // glTF MR：G=roughness，B=metallic
      mrOut[i + 0] = 255;
      mrOut[i + 1] = (unsigned char)(roughness * 255.0f + 0.5f);
      mrOut[i + 2] = (unsigned char)(metallic * 255.0f + 0.5f);
      mrOut[i + 3] = 255;
    }
  }

  mat->baseColorTexture = Texture::createFromRGBA(baseColorOut.data(), outW, outH, true);
  mat->metallicRoughnessTexture = Texture::createFromRGBA(mrOut.data(), outW, outH, false);
  mat->baseColorFactor[0] = mat->baseColorFactor[1] = mat->baseColorFactor[2] = 1.0f;
  mat->baseColorFactor[3] = 1.0f;
  mat->metallicFactor = 1.0f;
  mat->roughnessFactor = 1.0f;

  std::fprintf(stderr, "GltfLoader: converted SpecGloss→MR (%dx%d) for '%s'\n",
         outW, outH, mat->name.c_str());
  return true;
}

static void parseMaterialExtensions(const tinygltf::Material &gm,
                                    const tinygltf::Model &model,
                                    std::string const& basePath,
                                    PBRMaterial *mat) {
  tinygltf::ExtensionMap::const_iterator it;

  it = gm.extensions.find("KHR_materials_transmission");
  if (it != gm.extensions.end()) {
    const tinygltf::Value &ext = it->second;
    mat->hasTransmission = true;
    mat->ior = 1.5f;
    if (ext.Has("transmissionFactor"))
      mat->transmissionFactor = static_cast<float>(ext.Get("transmissionFactor").GetNumberAsDouble());
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
      mat->ior = static_cast<float>(ext.Get("ior").GetNumberAsDouble());
      mat->hasIor = true;
    }
  }

  it = gm.extensions.find("KHR_materials_volume");
  if (it != gm.extensions.end()) {
    const tinygltf::Value &ext = it->second;
    mat->hasVolume = true;
    if (ext.Has("thicknessFactor"))
      mat->volumeThickness = static_cast<float>(ext.Get("thicknessFactor").GetNumberAsDouble());
    if (ext.Has("attenuationDistance"))
      mat->volumeAttenuationDistance = static_cast<float>(ext.Get("attenuationDistance").GetNumberAsDouble());
    if (ext.Has("attenuationColor")) {
      const tinygltf::Value &c = ext.Get("attenuationColor");
      if (c.IsArray() && c.ArrayLen() >= 3) {
        mat->volumeAttenuationColor[0] = static_cast<float>(c.Get(0).GetNumberAsDouble());
        mat->volumeAttenuationColor[1] = static_cast<float>(c.Get(1).GetNumberAsDouble());
        mat->volumeAttenuationColor[2] = static_cast<float>(c.Get(2).GetNumberAsDouble());
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
      mat->clearcoatFactor = static_cast<float>(ext.Get("clearcoatFactor").GetNumberAsDouble());
    if (ext.Has("clearcoatRoughnessFactor"))
      mat->clearcoatRoughnessFactor = static_cast<float>(ext.Get("clearcoatRoughnessFactor").GetNumberAsDouble());
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
        mat->clearcoatNormalScale = static_cast<float>(tex.Get("scale").GetNumberAsDouble());
    }
  }
}

GltfLoader::GltfLoader()
  : model(nullptr), loader(nullptr), boundsValid(false) {}

GltfLoader::~GltfLoader() {
  destroy();
}

bool GltfLoader::load(std::string const& gltfPath) {
  destroy();

  model = new tinygltf::Model();
  loader = new tinygltf::TinyGLTF();

  basePath = getDirectory(gltfPath);
  std::string err, warn;

  bool ok = loader->LoadASCIIFromFile(model, &err, &warn, gltfPath);
  if (!warn.empty()) std::fprintf(stderr, "GltfLoader warn: %s\n", warn.c_str());
  if (!ok) {
    std::fprintf(stderr, "GltfLoader error: %s\n", err.c_str());
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
        std::vector<int> &ch = model->nodes[j].children;
        for (size_t k = 0; k < ch.size(); k++) {
          if (ch[k] == (int)i) { isChild = true; break; }
        }
      }
      if (!isChild) traverseNode((int)i, identity);
    }
  }

  if (meshes.empty()) {
    std::fprintf(stderr, "GltfLoader: no meshes found in %s\n", gltfPath.c_str());
    destroy();
    return false;
  }

  std::printf("GltfLoader (tinygltf): %u mesh(es), %u material(s), scene %d\n",
         static_cast<unsigned int>(meshes.size()), static_cast<unsigned int>(materials.size()), model->defaultScene);
  return true;
}

PBRMaterial *GltfLoader::buildMaterial(int materialIndex) {
  if (materialIndex < 0) return nullptr;
  if (materialIndex < static_cast<int>(materials.size()) && materials[materialIndex] != nullptr)
    return materials[materialIndex];

  while (static_cast<int>(materials.size()) <= materialIndex) materials.push_back(nullptr);
  if (materialIndex >= (int)model->materials.size()) return nullptr;

  const tinygltf::Material &gm = model->materials[materialIndex];
  PBRMaterial *mat = new PBRMaterial();
  mat->name = gm.name;
  mat->metallicFactor = (float)gm.pbrMetallicRoughness.metallicFactor;
  mat->roughnessFactor = (float)gm.pbrMetallicRoughness.roughnessFactor;
  mat->normalScale = (float)gm.normalTexture.scale;
  mat->occlusionStrength = (float)gm.occlusionTexture.strength;
  mat->alphaCutoff = (float)gm.alphaCutoff;
  mat->doubleSided = gm.doubleSided;

  std::vector<double> const& bcf = gm.pbrMetallicRoughness.baseColorFactor;
  for (int i = 0; i < 4; i++)
    mat->baseColorFactor[i] = i < static_cast<int>(bcf.size()) ? static_cast<float>(bcf[i]) : 1.0f;
  for (int i = 0; i < 3; i++)
    mat->emissiveFactor[i] = i < static_cast<int>(gm.emissiveFactor.size()) ? static_cast<float>(gm.emissiveFactor[i]) : 0.0f;

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

  applySpecularGlossinessConversion(gm, *model, basePath, mat);

  parseMaterialExtensions(gm, *model, basePath, mat);
  materials[materialIndex] = mat;
  return mat;
}

void GltfLoader::computeTangents(std::vector<Vertex> &vertices,
                                 std::vector<unsigned int> const& indices) {
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

    Vec3f const& p0 = vertices[i0].position;
    Vec3f const& p1 = vertices[i1].position;
    Vec3f const& p2 = vertices[i2].position;
    float u0 = vertices[i0].texCoord0.x(), v0 = vertices[i0].texCoord0.y();
    float u1 = vertices[i1].texCoord0.x(), v1 = vertices[i1].texCoord0.y();
    float u2 = vertices[i2].texCoord0.x(), v2 = vertices[i2].texCoord0.y();

    float x1 = p1.x() - p0.x(), x2 = p2.x() - p0.x();
    float y1 = p1.y() - p0.y(), y2 = p2.y() - p0.y();
    float z1 = p1.z() - p0.z(), z2 = p2.z() - p0.z();
    float s1 = u1 - u0, s2 = u2 - u0;
    float t1 = v1 - v0, t2 = v2 - v0;
    float denom = s1 * t2 - s2 * t1;
    if (std::fabs(denom) < 1e-8f) continue;
    float r = 1.0f / denom;

    Vec3f sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
    Vec3f tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);
    tan1[i0] += sdir; tan1[i1] += sdir; tan1[i2] += sdir;
    tan2[i0] += tdir; tan2[i1] += tdir; tan2[i2] += tdir;
  }

  for (size_t i = 0; i < vertices.size(); i++) {
    Vec3f const& n = vertices[i].normal;
    Vec3f t = tan1[i];
    Vec3f tangent = t - n * n.Dot3(t);
    tangent.Normalize();
    Vec3f bitangent;
    Vec3f::Cross3(bitangent, n, tangent);
    float w = (bitangent.Dot3(tan2[i]) < 0.0f) ? -1.0f : 1.0f;
    vertices[i].tangent.Set(tangent.x(), tangent.y(), tangent.z(), w);
  }
}

void GltfLoader::expandBounds(Vec3f const& p) {
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
                                     Matrix const& worldMatrix) {
  std::map<std::string, int>::const_iterator itPos = prim.attributes.find("POSITION");
  if (itPos == prim.attributes.end()) return nullptr;

  int posAcc = itPos->second;
  if (posAcc < 0 || posAcc >= (int)model->accessors.size()) return nullptr;
  size_t vertexCount = model->accessors[posAcc].count;

  std::vector<float> positions, normals, tangents, uvs;
  if (!readFloatAttribute(*model, posAcc, TINYGLTF_TYPE_VEC3, 3, positions))
    return nullptr;

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
    if (!readIndices(*model, prim.indices, indices)) return nullptr;
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

void GltfLoader::traverseNode(int nodeIndex, Matrix const& parentWorld) {
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
  model = nullptr;
  delete loader;
  loader = nullptr;

  basePath.clear();
  boundsValid = false;
}
