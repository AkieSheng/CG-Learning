#ifndef _SPHERE_H_
#define _SPHERE_H_

#include "object3d.h"
#include "vectors.h"

// 球体图元
class Sphere : public Object3D {

public:
  Sphere(Vec3f center, float radius, Material *m)
    : center(center), radius(radius) {
    material = m;
  }

  virtual bool intersect(const Ray &r, Hit &h, float tmin);
  virtual bool intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                               Material **outMaterial);
  virtual void paint(void) const;

private:
  Vec3f center;  // 球心
  float radius;  // 半径
};

#endif
