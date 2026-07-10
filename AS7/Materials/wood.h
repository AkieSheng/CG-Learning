#ifndef _WOOD_H_
#define _WOOD_H_

#include "material.h"

class Matrix;

// 木纹程序化材质
class Wood : public Material {

public:
  Wood(Matrix *m, Material *mat1, Material *mat2, int octaves,
       float frequency, float amplitude);
  virtual ~Wood();

  virtual Vec3f getDiffuseColor(const Vec3f &point) const;
  virtual Vec3f getSpecularColor(const Vec3f &point) const;
  virtual float getExponent(const Vec3f &point) const;
  virtual Vec3f getReflectiveColor(const Vec3f &point) const;
  virtual Vec3f getTransparentColor(const Vec3f &point) const;
  virtual float getIndexOfRefraction(const Vec3f &point) const;

  virtual Vec3f Shade(const Ray &ray, const Hit &hit,
                      const Vec3f &dirToLight,
                      const Vec3f &lightColor) const;

private:
  float blendWeight(const Vec3f &worldPoint) const; // 混合权重

  Matrix *mapping; // 映射矩阵
  Material *mat1; // 材质1
  Material *mat2; // 材质2
  int octaves; // 八度
  float frequency; // 频率
  float amplitude; // 振幅

  friend class Checkerboard;  // 访问 blendWeight
};

#endif
