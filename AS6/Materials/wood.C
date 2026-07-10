#include "wood.h"
#include "procedural_utils.h"
#include "matrix.h"

Wood::Wood(Matrix *m, Material *m1, Material *m2, int _octaves,
           float _frequency, float _amplitude)
    : mapping(m), mat1(m1), mat2(m2), octaves(_octaves),
      frequency(_frequency), amplitude(_amplitude) {}

Wood::~Wood() {
  delete mapping;
}

// 混合权重，采用圆柱坐标系
float Wood::blendWeight(const Vec3f &worldPoint) const {
  Vec3f p = mapToTextureSpace(mapping, worldPoint);
  return woodBlendWeight(p, octaves, frequency, amplitude);
}

Vec3f Wood::getDiffuseColor(const Vec3f &point) const {
  float t = blendWeight(point);
  return lerpVec3(mat1->getDiffuseColor(point), mat2->getDiffuseColor(point), t);
}

Vec3f Wood::getSpecularColor(const Vec3f &point) const {
  float t = blendWeight(point);
  return lerpVec3(mat1->getSpecularColor(point), mat2->getSpecularColor(point), t);
}

float Wood::getExponent(const Vec3f &point) const {
  float t = blendWeight(point);
  return lerpFloat(mat1->getExponent(point), mat2->getExponent(point), t);
}

Vec3f Wood::getReflectiveColor(const Vec3f &point) const {
  float t = blendWeight(point);
  return lerpVec3(mat1->getReflectiveColor(point), mat2->getReflectiveColor(point), t);
}

Vec3f Wood::getTransparentColor(const Vec3f &point) const {
  float t = blendWeight(point);
  return lerpVec3(mat1->getTransparentColor(point), mat2->getTransparentColor(point), t);
}

float Wood::getIndexOfRefraction(const Vec3f &point) const {
  float t = blendWeight(point);
  return lerpFloat(mat1->getIndexOfRefraction(point), mat2->getIndexOfRefraction(point), t);
}

Vec3f Wood::Shade(const Ray &ray, const Hit &hit,
                  const Vec3f &dirToLight,
                  const Vec3f &lightColor) const {
  float t = blendWeight(hit.getIntersectionPoint());
  Vec3f s1 = mat1->Shade(ray, hit, dirToLight, lightColor);
  Vec3f s2 = mat2->Shade(ray, hit, dirToLight, lightColor);
  return lerpVec3(s1, s2, t);
}

void Wood::glSetMaterial(void) const {
  mat1->glSetMaterial();
}
