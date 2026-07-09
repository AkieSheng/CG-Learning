#ifndef _NOISE_MATERIAL_H_
#define _NOISE_MATERIAL_H_

#include "material.h"

class Matrix;

// Perlin 噪声混合程序化材质
class Noise : public Material {

public:
  Noise(Matrix *m, Material *mat1, Material *mat2, int octaves);
  virtual ~Noise();

  virtual Vec3f getDiffuseColor(const Vec3f &point) const;
  virtual Vec3f getSpecularColor(const Vec3f &point) const;
  virtual float getExponent(const Vec3f &point) const;
  virtual Vec3f getReflectiveColor(const Vec3f &point) const;
  virtual Vec3f getTransparentColor(const Vec3f &point) const;
  virtual float getIndexOfRefraction(const Vec3f &point) const;

  virtual Vec3f Shade(const Ray &ray, const Hit &hit,
                      const Vec3f &dirToLight,
                      const Vec3f &lightColor) const;
  virtual void glSetMaterial(void) const;

private:
  float blendWeight(const Vec3f &worldPoint) const; // 混合权重

  Matrix *mapping; // 映射矩阵
  Material *mat1; // 材质1
  Material *mat2; // 材质2
  int octaves; // 八度
};

#endif
