#ifndef _SPHERE_H_
#define _SPHERE_H_

#include "object3d.h"
#include "vectors.h"

class Grid;
class Matrix;

// 球体图元
class Sphere : public Object3D {

public:
  Sphere(Vec3f center, float radius, Material *m);

  virtual bool intersect(const Ray &r, Hit &h, float tmin);
  virtual bool intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                               Material **outMaterial);
  virtual void insertIntoGrid(Grid *g, Matrix *m);

private:
  Vec3f center;  // 球心
  float radius;  // 半径
};

#endif
