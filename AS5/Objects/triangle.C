#include "triangle.h"
#include "grid.h"
#include "boundingbox.h"
#include "matrix.h"
#include "gl_headers.h"
#include <math.h>
#include <stdio.h>

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

// [DEBUG] 打印当前节点包围盒
void Triangle::debugPrintBoundingBox(int depth) const {
  for (int i = 0; i < depth; i++)
    printf("  ");
  printf("Triangle: ");
  if (bbox == NULL)
    printf("NULL bounding box\n");
  else
    bbox->Print();
}

// 由三顶点计算 AABB
void Triangle::computeWorldBounds(const Matrix *m, Vec3f &wmin,
                                  Vec3f &wmax) const {
  Vec3f wa = a, wb = b, wc = c;
  if (m != NULL) {
    m->Transform(wa);
    m->Transform(wb);
    m->Transform(wc);
  }

  wmin = Vec3f(fminf(fminf(wa.x(), wb.x()), wc.x()),
               fminf(fminf(wa.y(), wb.y()), wc.y()),
               fminf(fminf(wa.z(), wb.z()), wc.z()));
  wmax = Vec3f(fmaxf(fmaxf(wa.x(), wb.x()), wc.x()),
               fmaxf(fmaxf(wa.y(), wb.y()), wc.y()),
               fmaxf(fmaxf(wa.z(), wb.z()), wc.z()));
}

// 将图元栅格化
void Triangle::insertIntoGrid(Grid *g, Matrix *m) {
  if (g == NULL)
    return;

  Vec3f wmin, wmax;
  computeWorldBounds(m, wmin, wmax);
  BoundingBox worldBox(wmin, wmax);
  g->insertObjectInBBox(&worldBox, this, NULL);
}

// 射线-三角形求交（使用 Möller–Trumbore 算法）
// 参考：Möller & Trumbore, "Fast, Minimum Storage Ray/Triangle Intersection", 1997
bool Triangle::intersect(const Ray &r, Hit &h, float tmin) {
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
