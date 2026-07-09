#include "marble.h"
#include "procedural_utils.h"
#include "matrix.h"

// 构造大理石纹理程序化材质
Marble::Marble(Matrix *m, Material *m1, Material *m2, int _octaves,
               float _frequency, float _amplitude)
    : mapping(m), mat1(m1), mat2(m2), octaves(_octaves),
      frequency(_frequency), amplitude(_amplitude) {}

Marble::~Marble() {
  delete mapping;
}

// 混合权重
float Marble::blendWeight(const Vec3f &worldPoint) const {
  Vec3f p = mapToTextureSpace(mapping, worldPoint); // 映射到纹理空间
  double n = fractalNoise(p, octaves); // 噪声
  float v = sinf(frequency * p.x() + amplitude * (float)n);  // sin(freq·x + amp·N) 大理石纹路效果
  return clamp01(v * 0.5f + 0.5f);  // 归一化到[0,1]
}

Vec3f Marble::getDiffuseColor(const Vec3f &point) const {
  float t = blendWeight(point);
  return lerpVec3(mat1->getDiffuseColor(point), mat2->getDiffuseColor(point), t);
}

Vec3f Marble::getSpecularColor(const Vec3f &point) const {
  float t = blendWeight(point);
  return lerpVec3(mat1->getSpecularColor(point), mat2->getSpecularColor(point), t);
}

float Marble::getExponent(const Vec3f &point) const {
  float t = blendWeight(point);
  return lerpFloat(mat1->getExponent(point), mat2->getExponent(point), t);
}

Vec3f Marble::getReflectiveColor(const Vec3f &point) const {
  float t = blendWeight(point);
  return lerpVec3(mat1->getReflectiveColor(point), mat2->getReflectiveColor(point), t);
}

Vec3f Marble::getTransparentColor(const Vec3f &point) const {
  float t = blendWeight(point);
  return lerpVec3(mat1->getTransparentColor(point), mat2->getTransparentColor(point), t);
}

float Marble::getIndexOfRefraction(const Vec3f &point) const {
  float t = blendWeight(point);
  return lerpFloat(mat1->getIndexOfRefraction(point), mat2->getIndexOfRefraction(point), t);
}

Vec3f Marble::Shade(const Ray &ray, const Hit &hit,
                    const Vec3f &dirToLight,
                    const Vec3f &lightColor) const {
  float t = blendWeight(hit.getIntersectionPoint());
  Vec3f s1 = mat1->Shade(ray, hit, dirToLight, lightColor);
  Vec3f s2 = mat2->Shade(ray, hit, dirToLight, lightColor);
  return lerpVec3(s1, s2, t);
}

void Marble::glSetMaterial(void) const {
  mat1->glSetMaterial();
}
