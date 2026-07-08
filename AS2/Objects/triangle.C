#include "triangle.h"
#include <math.h>

// 射线-三角形求交（使用 Möller–Trumbore 算法）
// 参考：Möller & Trumbore, "Fast, Minimum Storage Ray/Triangle Intersection", 1997
bool Triangle::intersect(const Ray &r, Hit &h, float tmin) {
  Vec3f edge1 = b - a;  // 三角形边向量
  Vec3f edge2 = c - a;  // 三角形边向量
  Vec3f pvec;  // 叉积结果
  Vec3f::Cross3(pvec, r.getDirection(), edge2);  // 计算叉积
  float det = edge1.Dot3(pvec);  // 有向体积的 2 倍
  if (fabs(det) < 1e-8f)
    return false;  // 射线与三角形平面平行

  float invDet = 1.0f / det;  // 逆行列式
  Vec3f tvec = r.getOrigin() - a;  // 射线起点到三角形顶点的向量
  float u = tvec.Dot3(pvec) * invDet;  // 重心坐标 u
  if (u < 0.0f || u > 1.0f)
    return false;  // 重心坐标 u 不在三角形内

  Vec3f qvec;  // 叉积结果
  Vec3f::Cross3(qvec, tvec, edge1);  // 计算叉积
  float v = r.getDirection().Dot3(qvec) * invDet;  // 重心坐标 v
  if (v < 0.0f || u + v > 1.0f)
    return false;  // 重心坐标 v 不在三角形内

  float t = edge2.Dot3(qvec) * invDet;  // 交点参数 t
  if (t >= tmin && t < h.getT()) {  // 判断交点是否在 tmin 和当前最近交点之间
    h.set(t, material, normal, r);  // 更新 Hit
    return true;  // 相交
  }
  return false;  // 不相交
}
