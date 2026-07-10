#include "checkerboard.h"
#include "procedural_utils.h"
#include "matrix.h"

Checkerboard::Checkerboard(Matrix *m, Material *m1, Material *m2)
    : mapping(m), mat1(m1), mat2(m2) {}

Checkerboard::~Checkerboard() {
  delete mapping;
}

// 根据世界点选择材质
Material *Checkerboard::selectMaterial(const Vec3f &worldPoint) const {
  Vec3f p = mapToTextureSpace(mapping, worldPoint);
  p = scaleTex(p, checkerExtraScale(approxMappingScale(mapping))); // 缩放纹理
  int ix = (int)floor(p.x());
  int iy = (int)floor(p.y());
  int iz = (int)floor(p.z());
  if (procOdd(ix) ^ procOdd(iy) ^ procOdd(iz))
    return mat2;
  return mat1;
}

Vec3f Checkerboard::getDiffuseColor(const Vec3f &point) const {
  return selectMaterial(point)->getDiffuseColor(point);
}

Vec3f Checkerboard::getSpecularColor(const Vec3f &point) const {
  return selectMaterial(point)->getSpecularColor(point);
}

float Checkerboard::getExponent(const Vec3f &point) const {
  return selectMaterial(point)->getExponent(point);
}

Vec3f Checkerboard::getReflectiveColor(const Vec3f &point) const {
  return selectMaterial(point)->getReflectiveColor(point);
}

Vec3f Checkerboard::getTransparentColor(const Vec3f &point) const {
  return selectMaterial(point)->getTransparentColor(point);
}

float Checkerboard::getIndexOfRefraction(const Vec3f &point) const {
  return selectMaterial(point)->getIndexOfRefraction(point);
}

Vec3f Checkerboard::Shade(const Ray &ray, const Hit &hit,
                          const Vec3f &dirToLight,
                          const Vec3f &lightColor) const {
  return selectMaterial(hit.getIntersectionPoint())
      ->Shade(ray, hit, dirToLight, lightColor);
}

void Checkerboard::glSetMaterial(void) const {
  mat1->glSetMaterial();
}
