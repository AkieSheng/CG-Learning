#ifndef _OBJECT3D_H_
#define _OBJECT3D_H_

#include "material.h"
#include "ray.h"
#include "hit.h"

// 物体抽象基类
class Object3D {

public:
  Object3D() : material(NULL) {}
  virtual ~Object3D() {}

  virtual bool intersect(const Ray &r, Hit &h, float tmin) = 0;

protected:
  Material *material;  // 物体材质
};

#endif
