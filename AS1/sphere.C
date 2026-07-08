#include "sphere.h"
#include <math.h>

// 射线-球体求交
// 数学上：|O + tD - C|^2 = r^2 → 二次方程 a*t^2 + b*t + c = 0
// 参考：SIGGRAPH HyperGraph ray tracing notes; Shirley "Ray Tracing in One Weekend"
bool Sphere::intersect(const Ray &r, Hit &h, float tmin) {
  Vec3f oc = r.getOrigin() - center;  // 射线起点到球心的向量
  Vec3f dir = r.getDirection();  // 射线方向
  float a = dir.Dot3(dir);  // 射线方向的平方
  float b = 2.0f * oc.Dot3(dir);  // 射线起点到球心的向量与射线方向的点积
  float c = oc.Dot3(oc) - radius * radius;  // 射线起点到球心的向量的平方减去球的半径的平方
  float discriminant = b * b - 4.0f * a * c;  // 判别式
  if (discriminant < 0.0f)
    return false;  // 没有交点
  float sqrt_disc = sqrtf(discriminant);  // 判别式的平方根
  bool hit = false;  // 是否命中
  float t = (-b - sqrt_disc) / (2.0f * a);  // 近端交点
  if (t >= tmin && t < h.getT()) {  // 满足 t >= tmin 且 t < h.getT() 时更新 Hit
    h.set(t, material, r);  // 更新 Hit 时同时写 t 和 material
    hit = true;  // 命中
  }
  t = (-b + sqrt_disc) / (2.0f * a);  // 远端交点
  if (t >= tmin && t < h.getT()) {  // 满足 t >= tmin 且 t < h.getT() 时更新 Hit
    h.set(t, material, r);  // 更新 Hit 时同时写 t 和 material
    hit = true;  // 命中
  }

  return hit;
}
