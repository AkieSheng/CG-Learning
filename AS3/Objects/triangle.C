#include "triangle.h"
#include "gl_headers.h"
#include <math.h>

// 射线-三角形求交（使用 Möller–Trumbore 算法）
// 参考：Möller & Trumbore, "Fast, Minimum Storage Ray/Triangle Intersection", 1997
bool Triangle::intersect(const Ray &r, Hit &h, float tmin) {
  Vec3f edge1 = b - a;  // 三角形边向量
  Vec3f edge2 = c - a;
  Vec3f pvec;
  Vec3f::Cross3(pvec, r.getDirection(), edge2);  // pvec = D × E2
  float det = edge1.Dot3(pvec);  // 有向体积的 2 倍
  if (fabs(det) < 1e-8f)
    return false;  // 射线与三角形平面平行

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
  if (t >= tmin && t < h.getT()) {  // t 在 [tmin, h.getT()) 范围内
    h.set(t, material, normal, r);
    return true;  // 有交点
  }
  return false;
}

// OpenGL 绘制单个三角形
void Triangle::paint(void) const {
  if (material != NULL)
    material->glSetMaterial();

  glBegin(GL_TRIANGLES);
  glNormal3f(normal.x(), normal.y(), normal.z());
  glVertex3f(a.x(), a.y(), a.z());
  glVertex3f(b.x(), b.y(), b.z());
  glVertex3f(c.x(), c.y(), c.z());
  glEnd();
}
