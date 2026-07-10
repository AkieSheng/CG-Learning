#include "triangle.h"
#include "grid.h"
#include "boundingbox.h"
#include "matrix.h"
#include "raytracing_stats.h"
#include <math.h>

Triangle::Triangle(Vec3f a, Vec3f b, Vec3f c, Material *m)
    : a(a), b(b), c(c) {
  material = m;
  Vec3f::Cross3(normal, b - a, c - a);
  normal.Normalize();

  // 计算包围盒边界
  Vec3f bbMin(fminf(fminf(a.x(), b.x()), c.x()),
              fminf(fminf(a.y(), b.y()), c.y()),
              fminf(fminf(a.z(), b.z()), c.z()));
  Vec3f bbMax(fmaxf(fmaxf(a.x(), b.x()), c.x()),
              fmaxf(fmaxf(a.y(), b.y()), c.y()),
              fmaxf(fmaxf(a.z(), b.z()), c.z()));
  bbox = new BoundingBox(bbMin, bbMax);
}

// 将图元栅格化
// 有变换时先变换三个顶点，再取世界空间最小 AABB（special case）
void Triangle::insertIntoGrid(Grid *g, Matrix *m) {
  if (g == NULL)
    return;

  if (m == NULL) {
    Object3D::insertIntoGrid(g, NULL);
    return;
  }

  Vec3f v0 = a, v1 = b, v2 = c;
  m->Transform(v0);
  m->Transform(v1);
  m->Transform(v2);

  Vec3f wmin(fminf(fminf(v0.x(), v1.x()), v2.x()),
             fminf(fminf(v0.y(), v1.y()), v2.y()),
             fminf(fminf(v0.z(), v1.z()), v2.z()));
  Vec3f wmax(fmaxf(fmaxf(v0.x(), v1.x()), v2.x()),
             fmaxf(fmaxf(v0.y(), v1.y()), v2.y()),
             fmaxf(fmaxf(v0.z(), v1.z()), v2.z()));

  g->insertObjectInWorldAABB(wmin, wmax, this, m);
}

// 射线-三角形求交（使用 Möller–Trumbore 算法）
// 参考：Möller & Trumbore, "Fast, Minimum Storage Ray/Triangle Intersection", 1997
bool Triangle::intersect(const Ray &r, Hit &h, float tmin) {
  RayTracingStats::IncrementNumIntersections();
  // 计算三角形边向量
  Vec3f edge1 = b - a;
  Vec3f edge2 = c - a;
  Vec3f pvec;
  Vec3f::Cross3(pvec, r.getDirection(), edge2);  // pvec = D × E2
  float det = edge1.Dot3(pvec);  // 有向体积的 2 倍
  if (fabs(det) < 1e-8f)
    return false;  // 射线与三角形平面不相交

  float invDet = 1.0f / det;  // 逆行列式
  Vec3f tvec = r.getOrigin() - a;  // 射线起点到三角形顶点 a 的向量
  float u = tvec.Dot3(pvec) * invDet;  // 重心坐标 u
  if (u < 0.0f || u > 1.0f)
    return false;  // u 不在 [0, 1] 范围内，不在三角形内

  Vec3f qvec;
  Vec3f::Cross3(qvec, tvec, edge1);  // qvec = T × E1
  float v = r.getDirection().Dot3(qvec) * invDet;  // 重心坐标 v
  if (v < 0.0f || u + v > 1.0f)
    return false;  // v 不在 [0, 1-u] 范围内，不在三角形内

  float t = edge2.Dot3(qvec) * invDet;  // t = (E2·Q) / det
  if (t >= tmin && t < h.getT()) {
    h.set(t, material, normal, r);
    return true;
  }
  return false; // 无交点
}

// 阴影射线求交
bool Triangle::intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                               Material **outMaterial) {
  
  // 计算三角形边向量
  Vec3f edge1 = b - a; 
  Vec3f edge2 = c - a;
  Vec3f pvec;
  Vec3f::Cross3(pvec, r.getDirection(), edge2); // pvec = D × E2
  float det = edge1.Dot3(pvec); // 有向体积的 2 倍
  if (fabs(det) < 1e-8f)
    return false; // 射线与三角形平面不相交

  float invDet = 1.0f / det; // 逆行列式
  Vec3f tvec = r.getOrigin() - a; // 射线起点到三角形顶点 a 的向量
  float u = tvec.Dot3(pvec) * invDet; // 重心坐标 u
  if (u < 0.0f || u > 1.0f)
    return false; // u 不在 [0, 1] 范围内，即不在三角形内

  Vec3f qvec;
  Vec3f::Cross3(qvec, tvec, edge1); // qvec = T × E1
  float v = r.getDirection().Dot3(qvec) * invDet; // 重心坐标 v
  if (v < 0.0f || u + v > 1.0f)
    return false; // v 不在 [0, 1-u] 范围内，即不在三角形内

  float hitT = edge2.Dot3(qvec) * invDet; // t = (E2·Q) / det
  if (hitT >= tmin && hitT <= tmax) { 
    t = hitT;
    if (outMaterial != NULL)
      *outMaterial = material;
    return true;
  }
  return false; // 无交点
}
