#include "noise.h"
#include "procedural_utils.h"
#include "matrix.h"

Noise::Noise(Matrix *m, Material *m1, Material *m2, int _octaves)
    : mapping(m), mat1(m1), mat2(m2), octaves(_octaves) {}

Noise::~Noise() {
  delete mapping;
}


auto Noise::blendWeight(Vec3f const&worldPoint)const -> float {
  Vec3f p = mapToTextureSpace(mapping, worldPoint);
  return noiseToUnit(fractalNoise(p, octaves));
}

auto Noise::getDiffuseColor(Vec3f const&point)const -> Vec3f {
  float t = blendWeight(point);
  return lerpVec3(mat1->getDiffuseColor(point), mat2->getDiffuseColor(point), t);
}

auto Noise::getSpecularColor(Vec3f const&point)const -> Vec3f {
  float t = blendWeight(point);
  return lerpVec3(mat1->getSpecularColor(point), mat2->getSpecularColor(point), t);
}

auto Noise::getExponent(Vec3f const&point)const -> float {
  float t = blendWeight(point);
  return lerpFloat(mat1->getExponent(point), mat2->getExponent(point), t);
}

auto Noise::getReflectiveColor(Vec3f const&point)const -> Vec3f {
  float t = blendWeight(point);
  return lerpVec3(mat1->getReflectiveColor(point), mat2->getReflectiveColor(point), t);
}

auto Noise::getTransparentColor(Vec3f const&point)const -> Vec3f {
  float t = blendWeight(point);
  return lerpVec3(mat1->getTransparentColor(point), mat2->getTransparentColor(point), t);
}

auto Noise::getIndexOfRefraction(Vec3f const&point)const -> float {
  float t = blendWeight(point);
  return lerpFloat(mat1->getIndexOfRefraction(point), mat2->getIndexOfRefraction(point), t);
}

auto Noise::Shade(Ray const&ray, Hit const&hit,
                   Vec3f const&dirToLight,
                   Vec3f const&lightColor)const -> Vec3f {
  float t = blendWeight(hit.getIntersectionPoint());
  Vec3f s1 = mat1->Shade(ray, hit, dirToLight, lightColor);
  Vec3f s2 = mat2->Shade(ray, hit, dirToLight, lightColor);
  return lerpVec3(s1, s2, t);
}

auto Noise::glSetMaterial(void)const -> void {
  mat1->glSetMaterial();
}
