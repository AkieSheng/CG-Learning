#include "plane.h"
#include <math.h>

// 射线-平面求交
// 将 P(t)=O+tD 代入 P·n=d → t = (d - O·n) / (D·n)
bool Plane::intersect(const Ray &r, Hit &h, float tmin) {
  float denom = normal.Dot3(r.getDirection());  // 射线方向与平面法线点积
  if (fabs(denom) < 1e-6f)  // 容差
    return false;  // 射线与平面平行

  float t = (d - normal.Dot3(r.getOrigin())) / denom;  // 计算交点参数 t
  if (t >= tmin && t < h.getT()) {  // 判断交点是否在 tmin 和当前最近交点之间
    h.set(t, material, normal, r);  // 更新 Hit
    return true;  // 相交
  }
  return false;  // 不相交
}
