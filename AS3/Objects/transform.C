#include "transform.h"
#include "gl_headers.h"

// 变换求交：逆变换射线到局部空间，委托子物体求交，再变换法线回世界空间
// 法线规则：n_world = (M^-1)^T * n_local
bool Transform::intersect(const Ray &r, Hit &h, float tmin) {
  Matrix objectMatrix;  // 世界到局部的逆变换矩阵 M^-1
  matrix.Inverse(objectMatrix);  // 逆变换矩阵的转置

  Vec3f origin = r.getOrigin();  // 射线起点
  Vec3f direction = r.getDirection();  // 射线方向
  objectMatrix.Transform(origin);  // 变换起点
  objectMatrix.TransformDirection(direction);  // 变换方向

  Ray localRay(origin, direction);  // 变换后的射线
  Hit localHit(h);  // 变换后的交点
  if (!object->intersect(localRay, localHit, tmin))  // 委托子物体求交
    return false;  // 没有交点

  Vec3f normal = localHit.getNormal();  // 局部法线
  inverseMatrix.TransformDirection(normal);  // 变换法线
  normal.Normalize();

  h.set(localHit.getT(), localHit.getMaterial(), normal, r);  // 设置交点
  return true;
}

// OpenGL 绘制
void Transform::paint(void) const {
  glPushMatrix();  // 保存当前矩阵
  GLfloat *glMatrix = matrix.glGet();  // 获取变换矩阵
  glMultMatrixf(glMatrix);  // 应用变换矩阵
  delete[] glMatrix;  // 释放变换矩阵
  object->paint();  // 绘制子物体
  glPopMatrix();  // 恢复矩阵
}
