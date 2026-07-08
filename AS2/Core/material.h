#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "vectors.h"

// 材质
class Material {

public:
  Material(const Vec3f &d_color) { diffuseColor = d_color; }
  virtual ~Material() {}

  virtual Vec3f getDiffuseColor() const { return diffuseColor; }

protected:
  Vec3f diffuseColor;  // 漫反射颜色
};

#endif
