#include "plane.h"
#include "gl_headers.h"
#include <math.h>

// 射线-平面求交
// 将 P(t)=O+tD 代入 P·n=d → t = (d - O·n) / (D·n)
bool Plane::intersect(const Ray &r, Hit &h, float tmin) {
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

// OpenGL 绘制
void Plane::paint(void) const {
  if (material != NULL)
    material->glSetMaterial();

  // 选取与 n 不平行的辅助向量 v，构造平面内正交基 b1 = v×n, b2 = n×b1，用大矩形近似无限平面
  Vec3f v = (fabs(normal.x()) < 0.9f) ? Vec3f(1, 0, 0) : Vec3f(0, 1, 0);
  Vec3f b1, b2;
  Vec3f::Cross3(b1, v, normal);
  b1.Normalize();
  Vec3f::Cross3(b2, normal, b1);
  b2.Normalize();

  Vec3f origin = normal * d;  // 平面上距原点最近点 origin = n * d
  const float big = 1000.0f;  // 大矩形边长

  // 在 (b1, b2) 张成的 [-big, big]^2 矩形上绘制平面
  Vec3f p0 = origin + b1 * (-big) + b2 * (-big);
  Vec3f p1 = origin + b1 * big + b2 * (-big);
  Vec3f p2 = origin + b1 * big + b2 * big;
  Vec3f p3 = origin + b1 * (-big) + b2 * big;

  glBegin(GL_QUADS);  // 绘制四边形
  glNormal3f(normal.x(), normal.y(), normal.z());  // 法线
  // 顶点
  glVertex3f(p0.x(), p0.y(), p0.z());
  glVertex3f(p1.x(), p1.y(), p1.z());
  glVertex3f(p2.x(), p2.y(), p2.z());
  glVertex3f(p3.x(), p3.y(), p3.z());
  glEnd();
}
