#include "checkerboard.h"
#include "procedural_utils.h"
#include "wood.h"
#include "matrix.h"

const float Checkerboard::FLOOR_WOOD_FREQ_BOOST = 3.0f;

Checkerboard::Checkerboard(Matrix *m, Material *m1, Material *m2)
    : mapping(m), mat1(m1), mat2(m2)
    { }

Checkerboard::~Checkerboard()
{
  delete mapping;
}

Material *Checkerboard::selectMaterial(Vec3f const&worldPoint) const
{
  Vec3f p = mapToTextureSpace(mapping, worldPoint);
  int ix = static_cast<int>(::floor(p.x()));
  int iy = static_cast<int>(::floor(p.y()));
  int iz = static_cast<int>(::floor(p.z()));
  if (procOdd(ix) ^ procOdd(iy) ^ procOdd(iz))
    return mat2;
  return mat1;
}

Wood const*Checkerboard::selectedWood(Vec3f const&worldPoint) const
{
  return dynamic_cast<Wood const*>(selectMaterial(worldPoint));
}

float Checkerboard::woodCellWeight(Vec3f const&worldPoint,
                                   Wood const*wood) const {
  Vec3f p = mapToTextureSpace(mapping, worldPoint);
  return woodBlendWeight(p, wood->octaves, wood->frequency, wood->amplitude,
                         FLOOR_WOOD_FREQ_BOOST);
}

Vec3f Checkerboard::getDiffuseColor(Vec3f const&point) const
{
  Wood const*wood = selectedWood(point);
  if (wood == nullptr)
    return selectMaterial(point)->getDiffuseColor(point);
  float t = woodCellWeight(point, wood);
  return lerpVec3(wood->mat1->getDiffuseColor(point),
                  wood->mat2->getDiffuseColor(point), t);
}

Vec3f Checkerboard::getSpecularColor(Vec3f const&point) const
{
  Wood const*wood = selectedWood(point);
  if (wood == nullptr)
    return selectMaterial(point)->getSpecularColor(point);
  float t = woodCellWeight(point, wood);
  return lerpVec3(wood->mat1->getSpecularColor(point),
                  wood->mat2->getSpecularColor(point), t);
}

float Checkerboard::getExponent(Vec3f const&point) const
{
  Wood const*wood = selectedWood(point);
  if (wood == nullptr)
    return selectMaterial(point)->getExponent(point);
  float t = woodCellWeight(point, wood);
  return lerpFloat(wood->mat1->getExponent(point),
                   wood->mat2->getExponent(point), t);
}

Vec3f Checkerboard::getReflectiveColor(Vec3f const&point) const
{
  Wood const*wood = selectedWood(point);
  if (wood == nullptr)
    return selectMaterial(point)->getReflectiveColor(point);
  float t = woodCellWeight(point, wood);
  return lerpVec3(wood->mat1->getReflectiveColor(point),
                  wood->mat2->getReflectiveColor(point), t);
}

Vec3f Checkerboard::getTransparentColor(Vec3f const&point) const
{
  Wood const*wood = selectedWood(point);
  if (wood == nullptr)
    return selectMaterial(point)->getTransparentColor(point);
  float t = woodCellWeight(point, wood);
  return lerpVec3(wood->mat1->getTransparentColor(point),
                  wood->mat2->getTransparentColor(point), t);
}

float Checkerboard::getIndexOfRefraction(Vec3f const&point) const
{
  Wood const*wood = selectedWood(point);
  if (wood == nullptr)
    return selectMaterial(point)->getIndexOfRefraction(point);
  float t = woodCellWeight(point, wood);
  return lerpFloat(wood->mat1->getIndexOfRefraction(point),
                   wood->mat2->getIndexOfRefraction(point), t);
}

Vec3f Checkerboard::Shade(Ray const&ray, Hit const&hit,
                          Vec3f const&dirToLight,
                          Vec3f const&lightColor) const {
  Wood const*wood = selectedWood(hit.getIntersectionPoint());
  if (wood == nullptr)
    return selectMaterial(hit.getIntersectionPoint())
        ->Shade(ray, hit, dirToLight, lightColor);
  float t = woodCellWeight(hit.getIntersectionPoint(), wood);
  Vec3f s1 = wood->mat1->Shade(ray, hit, dirToLight, lightColor);
  Vec3f s2 = wood->mat2->Shade(ray, hit, dirToLight, lightColor);
  return lerpVec3(s1, s2, t);
}
