#include "marble.h"

#include "procedural_utils.h"
#include "matrix.h"


Marble::Marble(Matrix *m, Material *m1, Material *m2, int _octaves,
               float _frequency, float _amplitude)
    : mapping(m), mat1(m1), mat2(m2), octaves(_octaves),
      frequency(_frequency), amplitude(_amplitude) {}

Marble::~Marble() {
  delete mapping;
}


auto Marble::blendWeight(Vec3f const&worldPoint)const -> float {
  Vec3f p = mapToTextureSpace(mapping, worldPoint);
  double n = fractalNoise(p, octaves);
  float freq = frequency * marbleFrequencyScale();
  float v = ::sinf(freq * p.x() + amplitude * static_cast<float>(n));
  return clamp01(v * 0.5f + 0.5f);
}

auto Marble::getDiffuseColor(Vec3f const&point)const -> Vec3f {
  float t = blendWeight(point);
  return lerpVec3(mat1->getDiffuseColor(point), mat2->getDiffuseColor(point), t);
}

auto Marble::getSpecularColor(Vec3f const&point)const -> Vec3f {
  float t = blendWeight(point);
  return lerpVec3(mat1->getSpecularColor(point), mat2->getSpecularColor(point), t);
}

auto Marble::getExponent(Vec3f const&point)const -> float {
  float t = blendWeight(point);
  return lerpFloat(mat1->getExponent(point), mat2->getExponent(point), t);
}

auto Marble::getReflectiveColor(Vec3f const&point)const -> Vec3f {
  float t = blendWeight(point);
  return lerpVec3(mat1->getReflectiveColor(point), mat2->getReflectiveColor(point), t);
}

auto Marble::getTransparentColor(Vec3f const&point)const -> Vec3f {
  float t = blendWeight(point);
  return lerpVec3(mat1->getTransparentColor(point), mat2->getTransparentColor(point), t);
}

auto Marble::getIndexOfRefraction(Vec3f const&point)const -> float {
  float t = blendWeight(point);
  return lerpFloat(mat1->getIndexOfRefraction(point), mat2->getIndexOfRefraction(point), t);
}

auto Marble::Shade(Ray const&ray, Hit const&hit,
                    Vec3f const&dirToLight,
                    Vec3f const&lightColor)const -> Vec3f {
  float t = blendWeight(hit.getIntersectionPoint());
  Vec3f s1 = mat1->Shade(ray, hit, dirToLight, lightColor);
  Vec3f s2 = mat2->Shade(ray, hit, dirToLight, lightColor);
  return lerpVec3(s1, s2, t);
}

auto Marble::glSetMaterial(void)const -> void {
  mat1->glSetMaterial();
}
