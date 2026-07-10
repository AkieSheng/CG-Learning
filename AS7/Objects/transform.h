#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "object3d.h"
#include "matrix.h"

class Grid;

// 变换包装器
class Transform : public Object3D {

public:
  Transform(Matrix &m, Object3D *o);
  ~Transform();

  virtual bool intersect(const Ray &r, Hit &h, float tmin);
  virtual bool intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                               Material **outMaterial);
  virtual void insertIntoGrid(Grid *g, Matrix *m);

private:
  Matrix matrix;  // 物体到世界的变换矩阵 M
  Matrix inverseMatrix;  // 逆变换矩阵的转置 (M^-1)^T
  Object3D *object;  // 被包装的子物体 o
};

#endif
