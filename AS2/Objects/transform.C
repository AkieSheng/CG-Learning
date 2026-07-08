#include "transform.h"

// 变换求交：逆变换射线到局部空间，委托子物体求交，再变换法线回世界空间
// 法线规则：n_world = (M^-1)^T * n_local
bool Transform::intersect(const Ray &r, Hit &h, float tmin) {
  Matrix objectMatrix;  // 局部坐标系到世界坐标系的变换矩阵
  matrix.Inverse(objectMatrix);  // 世界到局部的逆变换矩阵

  Vec3f origin = r.getOrigin();  // 射线起点
  Vec3f direction = r.getDirection();  // 射线方向
  objectMatrix.Transform(origin);  // 变换射线起点到局部坐标系
  objectMatrix.TransformDirection(direction);  // 变换射线方向到局部坐标系

  Ray localRay(origin, direction);  // 局部坐标系下的射线
  Hit localHit(h);  // 局部坐标系下的 Hit
  if (!object->intersect(localRay, localHit, tmin))  // 委托子物体求交
    return false;  // 不相交

  Vec3f normal = localHit.getNormal();  // 局部坐标系下的法线
  inverseMatrix.TransformDirection(normal);  // 应用逆变换矩阵的转置（(M^-1)^T）
  normal.Normalize();  // 归一化

  h.set(localHit.getT(), localHit.getMaterial(), normal, r);  // 更新 Hit
  return true;  // 相交
}
