#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

#include "object3d.h"
#include "vectors.h"

class Grid;
class Matrix;

// 三角形图元
class Triangle : public Object3D {

public:
  Triangle(Vec3f a, Vec3f b, Vec3f c, Material *m);

  virtual bool intersect(const Ray &r, Hit &h, float tmin);
  virtual bool intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                               Material **outMaterial);
  virtual void insertIntoGrid(Grid *g, Matrix *m);

private:
  Vec3f a, b, c;  // 三个顶点
  Vec3f normal;   // 面法线
};

#endif
