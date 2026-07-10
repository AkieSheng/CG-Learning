#include "checkerboard.h"
#include "procedural_utils.h"
#include "wood.h"
#include "matrix.h"

const float Checkerboard::FLOOR_WOOD_FREQ_BOOST = 3.0f;

Checkerboard::Checkerboard(Matrix *m, Material *m1, Material *m2)
    : mapping(m), mat1(m1), mat2(m2) {}

Checkerboard::~Checkerboard() {
  delete mapping;
}

// 根据世界点选择材质
Material *Checkerboard::selectMaterial(const Vec3f &worldPoint) const {
  Vec3f p = mapToTextureSpace(mapping, worldPoint);
  int ix = (int)floor(p.x());
  int iy = (int)floor(p.y());
  int iz = (int)floor(p.z());
  if (procOdd(ix) ^ procOdd(iy) ^ procOdd(iz))
    return mat2;
  return mat1;
}

const Wood *Checkerboard::selectedWood(const Vec3f &worldPoint) const {
  return dynamic_cast<const Wood *>(selectMaterial(worldPoint));
}

// 地板格：带频率系数（FLOOR_WOOD_FREQ_BOOST）的 Checkerboard 映射
float Checkerboard::woodCellWeight(const Vec3f &worldPoint,
                                   const Wood *wood) const {
  Vec3f p = mapToTextureSpace(mapping, worldPoint);
  return woodBlendWeight(p, wood->octaves, wood->frequency, wood->amplitude,
                         FLOOR_WOOD_FREQ_BOOST);
}

Vec3f Checkerboard::getDiffuseColor(const Vec3f &point) const {
  const Wood *wood = selectedWood(point);
  if (wood == NULL)
    return selectMaterial(point)->getDiffuseColor(point);
  float t = woodCellWeight(point, wood);
  return lerpVec3(wood->mat1->getDiffuseColor(point),
                  wood->mat2->getDiffuseColor(point), t);
}

Vec3f Checkerboard::getSpecularColor(const Vec3f &point) const {
  const Wood *wood = selectedWood(point);
  if (wood == NULL)
    return selectMaterial(point)->getSpecularColor(point);
  float t = woodCellWeight(point, wood);
  return lerpVec3(wood->mat1->getSpecularColor(point),
                  wood->mat2->getSpecularColor(point), t);
}

float Checkerboard::getExponent(const Vec3f &point) const {
  const Wood *wood = selectedWood(point);
  if (wood == NULL)
    return selectMaterial(point)->getExponent(point);
  float t = woodCellWeight(point, wood);
  return lerpFloat(wood->mat1->getExponent(point),
                   wood->mat2->getExponent(point), t);
}

Vec3f Checkerboard::getReflectiveColor(const Vec3f &point) const {
  const Wood *wood = selectedWood(point);
  if (wood == NULL)
    return selectMaterial(point)->getReflectiveColor(point);
  float t = woodCellWeight(point, wood);
  return lerpVec3(wood->mat1->getReflectiveColor(point),
                  wood->mat2->getReflectiveColor(point), t);
}

Vec3f Checkerboard::getTransparentColor(const Vec3f &point) const {
  const Wood *wood = selectedWood(point);
  if (wood == NULL)
    return selectMaterial(point)->getTransparentColor(point);
  float t = woodCellWeight(point, wood);
  return lerpVec3(wood->mat1->getTransparentColor(point),
                  wood->mat2->getTransparentColor(point), t);
}

float Checkerboard::getIndexOfRefraction(const Vec3f &point) const {
  const Wood *wood = selectedWood(point);
  if (wood == NULL)
    return selectMaterial(point)->getIndexOfRefraction(point);
  float t = woodCellWeight(point, wood);
  return lerpFloat(wood->mat1->getIndexOfRefraction(point),
                     wood->mat2->getIndexOfRefraction(point), t);
}

Vec3f Checkerboard::Shade(const Ray &ray, const Hit &hit,
                          const Vec3f &dirToLight,
                          const Vec3f &lightColor) const {
  const Wood *wood = selectedWood(hit.getIntersectionPoint());
  if (wood == NULL)
    return selectMaterial(hit.getIntersectionPoint())
        ->Shade(ray, hit, dirToLight, lightColor);
  float t = woodCellWeight(hit.getIntersectionPoint(), wood);
  Vec3f s1 = wood->mat1->Shade(ray, hit, dirToLight, lightColor);
  Vec3f s2 = wood->mat2->Shade(ray, hit, dirToLight, lightColor);
  return lerpVec3(s1, s2, t);
}

void Checkerboard::glSetMaterial(void) const {
  mat1->glSetMaterial();
}
