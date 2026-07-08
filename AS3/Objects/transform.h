#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "object3d.h"
#include "matrix.h"

// 变换包装器
class Transform : public Object3D {

public:
  Transform(Matrix &m, Object3D *o)
    : matrix(m), object(o) {
    matrix.Inverse(inverseMatrix);
    inverseMatrix.Transpose();
  }
  ~Transform() { delete object; }

  virtual bool intersect(const Ray &r, Hit &h, float tmin);
  virtual void paint(void) const;

private:
  Matrix matrix;  // 物体到世界的变换矩阵 M
  Matrix inverseMatrix;  // 逆变换矩阵的转置 (M^-1)^T
  Object3D *object;  // 被包装的子物体 o
};

#endif
