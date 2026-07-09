#include "sphere.h"
#include "grid.h"
#include "boundingbox.h"
#include "matrix.h"
#include "gl_options.h"
#include "gl_headers.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Sphere::Sphere(Vec3f center, float radius, Material *m)
    : center(center), radius(radius) {
  material = m;
  Vec3f rVec(radius, radius, radius);
  bbox = new BoundingBox(center - rVec, center + rVec);  // 包围盒
}

// 将球体栅格化到体素网格
void Sphere::insertIntoGrid(Grid *g, Matrix *m) {
  if (g == NULL)
    return;

  // 计算测试半径
  float halfDiag = g->getVoxelHalfDiagonal();
  float testRadius = radius + halfDiag;

  // 计算逆变换矩阵
  Matrix inv;
  if (m != NULL)
    m->Inverse(inv);

  // 计算包围盒边界
  Vec3f rmin = bbox->getMin();
  Vec3f rmax = bbox->getMax();
  if (m != NULL) {
    // 计算包围盒8个顶点
    Vec3f corners[8] = {
      Vec3f(rmin.x(), rmin.y(), rmin.z()),
      Vec3f(rmax.x(), rmin.y(), rmin.z()),
      Vec3f(rmin.x(), rmax.y(), rmin.z()),
      Vec3f(rmax.x(), rmax.y(), rmin.z()),
      Vec3f(rmin.x(), rmin.y(), rmax.z()),
      Vec3f(rmax.x(), rmin.y(), rmax.z()),
      Vec3f(rmin.x(), rmax.y(), rmax.z()),
      Vec3f(rmax.x(), rmax.y(), rmax.z())
    };
    m->Transform(corners[0]);  // 变换矩阵
    rmin = corners[0];
    rmax = corners[0];
    for (int i = 1; i < 8; i++) {
      // 应用变换矩阵
      m->Transform(corners[i]);
      // 计算最小边界
      rmin = Vec3f(fminf(rmin.x(), corners[i].x()),
                   fminf(rmin.y(), corners[i].y()),
                   fminf(rmin.z(), corners[i].z()));
      // 计算最大边界
      rmax = Vec3f(fmaxf(rmax.x(), corners[i].x()),
                   fmaxf(rmax.y(), corners[i].y()),
                   fmaxf(rmax.z(), corners[i].z()));
    }
  }

  // 索引范围覆盖 testRadius 可达的体素
  Vec3f margin(halfDiag, halfDiag, halfDiag);
  rmin = rmin - margin;
  rmax = rmax + margin;

  // 计算体素索引范围
  Vec3f bbMin = g->getBoundingBox()->getMin();
  Vec3f bbMax = g->getBoundingBox()->getMax();
  float dx = (bbMax.x() - bbMin.x()) / g->getNX();
  float dy = (bbMax.y() - bbMin.y()) / g->getNY();
  float dz = (bbMax.z() - bbMin.z()) / g->getNZ();
  int i0 = (int)((rmin.x() - bbMin.x()) / dx);
  int i1 = (int)((rmax.x() - bbMin.x()) / dx);
  int j0 = (int)((rmin.y() - bbMin.y()) / dy);
  int j1 = (int)((rmax.y() - bbMin.y()) / dy);
  int k0 = (int)((rmin.z() - bbMin.z()) / dz);
  int k1 = (int)((rmax.z() - bbMin.z()) / dz);
  // 缩小到体素网格边界内
  if (i0 < 0) i0 = 0;
  if (j0 < 0) j0 = 0;
  if (k0 < 0) k0 = 0;
  if (i0 >= g->getNX()) i0 = g->getNX() - 1;
  if (j0 >= g->getNY()) j0 = g->getNY() - 1;
  if (k0 >= g->getNZ()) k0 = g->getNZ() - 1;
  if (i1 >= g->getNX()) i1 = g->getNX() - 1;
  if (j1 >= g->getNY()) j1 = g->getNY() - 1;
  if (k1 >= g->getNZ()) k1 = g->getNZ() - 1;

  // 遍历体素网格，将球体栅格化到体素网格
  for (int i = i0; i <= i1; i++) {
    for (int j = j0; j <= j1; j++) {
      for (int k = k0; k <= k1; k++) {
        Vec3f voxelCenter = g->getVoxelCenter(i, j, k);
        // 应用逆变换矩阵
        if (m != NULL)
          inv.Transform(voxelCenter);
        // 若体素中心到球心距离 <= 测试半径，则插入体素
        if ((voxelCenter - center).Length() <= testRadius)
          g->insertObject(i, j, k, this);
      }
    }
  }
}

// [DEBUG] 打印当前节点包围盒
void Sphere::debugPrintBoundingBox(int depth) const {
  for (int i = 0; i < depth; i++)
    printf("  ");
  printf("Sphere: ");
  if (bbox == NULL)
    printf("NULL bounding box\n");
  else
    bbox->Print();
}

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

// 阴影射线求交
bool Sphere::intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                             Material **outMaterial) {
  Vec3f oc = r.getOrigin() - center;  // 射线起点到球心的向量
  Vec3f dir = r.getDirection();
  float a = dir.Dot3(dir);  // D·D
  float b = 2.0f * oc.Dot3(dir);  // 2 * (O-C)·D
  float c = oc.Dot3(oc) - radius * radius;  // (O-C)·(O-C) - r^2
  float discriminant = b * b - 4.0f * a * c;  // b^2 - 4ac
  if (discriminant < 0.0f)
    return false;  // 无实根，不相交

  float sqrt_disc = sqrtf(discriminant);
  float tNear = (-b - sqrt_disc) / (2.0f * a);  // 近端交点
  float tFar = (-b + sqrt_disc) / (2.0f * a);  // 远端交点

  if (outMaterial == NULL) {
    if ((tNear >= tmin && tNear <= tmax) || (tFar >= tmin && tFar <= tmax)) {
      t = (tNear >= tmin && tNear <= tmax) ? tNear : tFar;  // 选择最近的交点
      return true;
    }
    return false;  // 无交点
  }

  // 寻找最近的交点并返回材质
  bool hit = false;
  float bestT = tmax;
  if (tNear >= tmin && tNear < bestT) {
    bestT = tNear;
    hit = true;
  }
  if (tFar >= tmin && tFar < bestT) {
    bestT = tFar;
    hit = true;
  }
  if (hit) {
    t = bestT;
    *outMaterial = material;
  }
  return hit;
}

// 球面参数化求点
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
