#ifndef _CHECKERBOARD_H_
#define _CHECKERBOARD_H_

#include "material.h"

class Matrix;
class Wood;

// 3D 棋盘格程序化材质
// 储存两个材质的指针，以及指向映射矩阵的指针
class Checkerboard : public Material {

public:
  Checkerboard(Matrix *m, Material *mat1, Material *mat2);
  virtual ~Checkerboard();

  virtual Vec3f getDiffuseColor(const Vec3f &point) const;
  virtual Vec3f getSpecularColor(const Vec3f &point) const;
  virtual float getExponent(const Vec3f &point) const;
  virtual Vec3f getReflectiveColor(const Vec3f &point) const;
  virtual Vec3f getTransparentColor(const Vec3f &point) const;
  virtual float getIndexOfRefraction(const Vec3f &point) const;

  virtual Vec3f Shade(const Ray &ray, const Hit &hit,
                      const Vec3f &dirToLight,
                      const Vec3f &lightColor) const; // 着色函数，委托到对应子材质

private:
  Material *selectMaterial(const Vec3f &worldPoint) const;
  const Wood *selectedWood(const Vec3f &worldPoint) const;
  float woodCellWeight(const Vec3f &worldPoint, const Wood *wood) const;

  static const float FLOOR_WOOD_FREQ_BOOST;  // 地板木纹频率系数

  Matrix *mapping;  // 映射矩阵
  Material *mat1;  // 材质1
  Material *mat2;  // 材质2
};

#endif
