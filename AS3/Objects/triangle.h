#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

#include "object3d.h"
#include "vectors.h"

// 三角形图元
class Triangle : public Object3D {

public:
  Triangle(Vec3f a, Vec3f b, Vec3f c, Material *m)
    : a(a), b(b), c(c) {
    material = m;
    Vec3f::Cross3(normal, b - a, c - a);  // 计算面法线
    normal.Normalize();  // 归一化
  }

  virtual bool intersect(const Ray &r, Hit &h, float tmin);
  virtual void paint(void) const;

private:
  Vec3f a, b, c;  // 三个顶点
  Vec3f normal;   // 面法线
};

#endif
