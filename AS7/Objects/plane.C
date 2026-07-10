#include "plane.h"
#include "grid.h"
#include "raytracing_stats.h"
#include <math.h>

Plane::Plane(Vec3f normal, float d, Material *m)
    : normal(normal), d(d) {
  material = m;
  normal.Normalize();
  bbox = NULL;
}

void Plane::insertIntoGrid(Grid *g, Matrix *m) {
  if (g != NULL)
    g->addInfiniteObject(this);
}

// 射线-平面求交
// 将 P(t)=O+tD 代入 P·n=d → t = (d - O·n) / (D·n)
bool Plane::intersect(const Ray &r, Hit &h, float tmin) {
  RayTracingStats::IncrementNumIntersections();
  float denom = normal.Dot3(r.getDirection());  // D·n
  if (fabs(denom) < 1e-6f)  // 容差
    return false;  // 射线与平面平行

  float t = (d - normal.Dot3(r.getOrigin())) / denom;  // t = (d - O·n) / (D·n)
  if (t >= tmin && t < h.getT()) {
    h.set(t, material, normal, r);  // 交点
    return true;
  }
  return false;
}

// 阴影射线求交
bool Plane::intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                            Material **outMaterial) {
  float denom = normal.Dot3(r.getDirection());  // D·n
  if (fabs(denom) < 1e-6f)  // 容差
    return false;

  // 计算交点t
  float hitT = (d - normal.Dot3(r.getOrigin())) / denom;  // t = (d - O·n) / (D·n)
  if (hitT >= tmin && hitT <= tmax) {
    t = hitT;
    if (outMaterial != NULL)
      *outMaterial = material;
    return true;
  }
  return false;
}
