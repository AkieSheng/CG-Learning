#include "sphere.h"
#include "gl_options.h"
#include "gl_headers.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 射线-球体求交
// 数学上：|O + tD - C|^2 = r^2 → 二次方程 a*t^2 + b*t + c = 0
bool Sphere::intersect(const Ray &r, Hit &h, float tmin) {
  Vec3f oc = r.getOrigin() - center;  // 射线起点到球心的向量
  Vec3f dir = r.getDirection();
  float a = dir.Dot3(dir);  // D·D
  float b = 2.0f * oc.Dot3(dir);  // 2 * (O-C)·D
  float c = oc.Dot3(oc) - radius * radius;  // (O-C)·(O-C) - r^2
  float discriminant = b * b - 4.0f * a * c;  // b^2 - 4ac
  if (discriminant < 0.0f)
    return false;  // 判别式 < 0，无实根，不相交

  float sqrt_disc = sqrtf(discriminant);
  bool hit = false;
  float t = (-b - sqrt_disc) / (2.0f * a);  // 近端交点
  if (t >= tmin && t < h.getT()) {
    // 法线：n = (P - C) / |P - C|
    Vec3f normal = r.pointAtParameter(t) - center;
    normal.Normalize();
    h.set(t, material, normal, r);
    hit = true;  // 有交点
  }
  t = (-b + sqrt_disc) / (2.0f * a);  // 远端交点
  if (t >= tmin && t < h.getT()) {
    Vec3f normal = r.pointAtParameter(t) - center;
    normal.Normalize();
    h.set(t, material, normal, r);
    hit = true;  // 有交点
  }
  return hit;
}

// 球面参数化求点：theta ∈ [0, 2π]，phi ∈ [-π/2, π/2]
// 球面坐标：x = cos(phi)cos(theta), y = sin(phi), z = cos(phi)sin(theta)
static Vec3f spherePoint(const Vec3f &center, float radius,
                         float theta, float phi) {
  float cosPhi = cosf(phi);
  Vec3f p(cosPhi * cosf(theta), sinf(phi), cosPhi * sinf(theta));
  return center + p * radius;
}

// OpenGL 绘制
void Sphere::paint(void) const {
  if (material != NULL)
    material->glSetMaterial();

  int thetaSteps = tessellation_theta;  // theta 方向步数
  int phiSteps = tessellation_phi;  // phi 方向步数
  // 最小步数
  if (thetaSteps < 2) thetaSteps = 2;
  if (phiSteps < 2) phiSteps = 2;
  // 步长
  float dTheta = 2.0f * (float)M_PI / thetaSteps;
  float dPhi = (float)M_PI / phiSteps;

  glBegin(GL_QUADS);
  for (int iPhi = 0; iPhi < phiSteps; iPhi++) {  // phi 方向循环
    float phi0 = -0.5f * (float)M_PI + iPhi * dPhi;
    float phi1 = phi0 + dPhi;
    for (int iTheta = 0; iTheta < thetaSteps; iTheta++) {  // theta 方向循环
      float theta0 = iTheta * dTheta;
      float theta1 = theta0 + dTheta;

      Vec3f v00 = spherePoint(center, radius, theta0, phi0);
      Vec3f v01 = spherePoint(center, radius, theta1, phi0);
      Vec3f v11 = spherePoint(center, radius, theta1, phi1);
      Vec3f v10 = spherePoint(center, radius, theta0, phi1);

      if (gouraud_shading) {  // Gouraud 着色
        // Gouraud：每顶点设置球面真法线 n = (P - C) / |P - C|
        Vec3f n00 = v00 - center; n00.Normalize();
        Vec3f n01 = v01 - center; n01.Normalize();
        Vec3f n11 = v11 - center; n11.Normalize();
        Vec3f n10 = v10 - center; n10.Normalize();

        // 绘制四边形
        glNormal3f(n00.x(), n00.y(), n00.z());
        glVertex3f(v00.x(), v00.y(), v00.z());
        glNormal3f(n01.x(), n01.y(), n01.z());
        glVertex3f(v01.x(), v01.y(), v01.z());
        glNormal3f(n11.x(), n11.y(), n11.z());
        glVertex3f(v11.x(), v11.y(), v11.z());
        glNormal3f(n10.x(), n10.y(), n10.z());
        glVertex3f(v10.x(), v10.y(), v10.z());
      } else {
        // 整片四边形共用一个面法线 faceNormal = normalize((v01-v00) × (v10-v00))
        Vec3f edge1 = v01 - v00;
        Vec3f edge2 = v10 - v00;
        Vec3f faceNormal;
        Vec3f::Cross3(faceNormal, edge1, edge2);
        faceNormal.Normalize();

        glNormal3f(faceNormal.x(), faceNormal.y(), faceNormal.z());  // 法线
        // 顶点
        glVertex3f(v00.x(), v00.y(), v00.z());
        glVertex3f(v01.x(), v01.y(), v01.z());
        glVertex3f(v11.x(), v11.y(), v11.z());
        glVertex3f(v10.x(), v10.y(), v10.z());
      }
    }
  }
  glEnd();
}
