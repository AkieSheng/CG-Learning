#include "sphere.h"
#include <math.h>

// 射线-球体求交
// 数学上：|O + tD - C|^2 = r^2 → 二次方程 a*t^2 + b*t + c = 0
bool Sphere::intersect(const Ray &r, Hit &h, float tmin) {
  Vec3f oc = r.getOrigin() - center;  // 射线起点到球心的向量
  Vec3f dir = r.getDirection();   // 射线方向
  float a = dir.Dot3(dir);  // D·D
  float b = 2.0f * oc.Dot3(dir);  // 2 * (O-C)·D
  float c = oc.Dot3(oc) - radius * radius;  // (O-C)·(O-C) - r^2
  float discriminant = b * b - 4.0f * a * c;  // b^2 - 4ac
  if (discriminant < 0.0f)
    return false;  // 判别式 < 0，无实根，不相交

  float sqrt_disc = sqrtf(discriminant);  // 判别式的平方根
  bool hit = false;
  float t = (-b - sqrt_disc) / (2.0f * a);  // 近端交点 t1 = (-b - sqrt(b^2 - 4ac)) / (2a)
  if (t >= tmin && t < h.getT()) {
    // 法线：交点指向球外侧 n = (P - C) / |P - C|
    Vec3f normal = r.pointAtParameter(t) - center;  // P - C
    normal.Normalize();  // 归一化
    h.set(t, material, normal, r);  // 更新 Hit
    hit = true;  // 相交
  }
  t = (-b + sqrt_disc) / (2.0f * a);  // 远端交点 t2 = (-b + sqrt(b^2 - 4ac)) / (2a)
  if (t >= tmin && t < h.getT()) {
    Vec3f normal = r.pointAtParameter(t) - center;  // P - C
    normal.Normalize();  // 归一化
    h.set(t, material, normal, r);  // 更新 Hit
    hit = true;  // 相交
  }
  return hit;
}
