#include "noise.h"
#include "procedural_utils.h"
#include "matrix.h"

Noise::Noise(Matrix *m, Material *m1, Material *m2, int _octaves)
    : mapping(m), mat1(m1), mat2(m2), octaves(_octaves) {}

Noise::~Noise() {
  delete mapping;
}

float Noise::blendWeight(Vec3f const&worldPoint) const {
  Vec3f p = mapToTextureSpace(mapping, worldPoint);
  return noiseToUnit(fractalNoise(p, octaves));
}

Vec3f Noise::getDiffuseColor(Vec3f const&point) const {
  float t = blendWeight(point);
  return lerpVec3(mat1->getDiffuseColor(point), mat2->getDiffuseColor(point), t);
}

Vec3f Noise::getSpecularColor(Vec3f const&point) const {
  float t = blendWeight(point);
  return lerpVec3(mat1->getSpecularColor(point), mat2->getSpecularColor(point), t);
}

float Noise::getExponent(Vec3f const&point) const {
  float t = blendWeight(point);
  return lerpFloat(mat1->getExponent(point), mat2->getExponent(point), t);
}

Vec3f Noise::getReflectiveColor(Vec3f const&point) const {
  float t = blendWeight(point);
  return lerpVec3(mat1->getReflectiveColor(point), mat2->getReflectiveColor(point), t);
}

Vec3f Noise::getTransparentColor(Vec3f const&point) const {
  float t = blendWeight(point);
  return lerpVec3(mat1->getTransparentColor(point), mat2->getTransparentColor(point), t);
}

float Noise::getIndexOfRefraction(Vec3f const&point) const {
  float t = blendWeight(point);
  return lerpFloat(mat1->getIndexOfRefraction(point), mat2->getIndexOfRefraction(point), t);
}

Vec3f Noise::Shade(Ray const&ray, Hit const&hit,
                   Vec3f const&dirToLight,
                   Vec3f const&lightColor) const {
  float t = blendWeight(hit.getIntersectionPoint());
  Vec3f s1 = mat1->Shade(ray, hit, dirToLight, lightColor);
  Vec3f s2 = mat2->Shade(ray, hit, dirToLight, lightColor);
  return lerpVec3(s1, s2, t);
}
