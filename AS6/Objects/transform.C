#include "transform.h"
#include "grid.h"
#include "boundingbox.h"
#include "gl_headers.h"
#include <math.h>
#include <stdio.h>

// 构造变换
Transform::Transform(Matrix &m, Object3D *o)
    : matrix(m), object(o) {
  matrix.Inverse(inverseMatrix);  // 逆变换矩阵
  inverseMatrix.Transpose();  // 逆变换矩阵的转置

  BoundingBox *childBox = object->getBoundingBox();  // 子物体包围盒
  // 计算包围盒边界
  if (childBox != NULL) {
    Vec3f cmin = childBox->getMin();
    Vec3f cmax = childBox->getMax();
    Vec3f corners[8] = {
      Vec3f(cmin.x(), cmin.y(), cmin.z()),
      Vec3f(cmax.x(), cmin.y(), cmin.z()),
      Vec3f(cmin.x(), cmax.y(), cmin.z()),
      Vec3f(cmax.x(), cmax.y(), cmin.z()),
      Vec3f(cmin.x(), cmin.y(), cmax.z()),
      Vec3f(cmax.x(), cmin.y(), cmax.z()),
      Vec3f(cmin.x(), cmax.y(), cmax.z()),
      Vec3f(cmax.x(), cmax.y(), cmax.z())
    };

    // 变换包围盒边界
    matrix.Transform(corners[0]);
    Vec3f worldMin = corners[0];
    Vec3f worldMax = corners[0];
    for (int i = 1; i < 8; i++) {
      matrix.Transform(corners[i]);  // 应用变换矩阵
      // 计算最小边界
      worldMin = Vec3f(fminf(worldMin.x(), corners[i].x()),
                       fminf(worldMin.y(), corners[i].y()),
                       fminf(worldMin.z(), corners[i].z()));
      // 计算最大边界
      worldMax = Vec3f(fmaxf(worldMax.x(), corners[i].x()),
                       fmaxf(worldMax.y(), corners[i].y()),
                       fmaxf(worldMax.z(), corners[i].z()));
    }
    bbox = new BoundingBox(worldMin, worldMax);
  }
}

Transform::~Transform() {
  delete object;
}

// 将图元栅格化
void Transform::insertIntoGrid(Grid *g, Matrix *m) {
  Matrix combined = matrix;
  if (m != NULL)
    combined = (*m) * matrix;
  object->insertIntoGrid(g, &combined);
}

// [DEBUG] 打印当前节点包围盒
void Transform::debugPrintBoundingBox(int depth) const {
  for (int i = 0; i < depth; i++)
    printf("  ");
  printf("Transform: ");
  if (bbox == NULL)
    printf("NULL bounding box\n");
  else
    bbox->Print();
  if (object != NULL)
    object->debugPrintBoundingBox(depth + 1);
}

// 射线求交：逆变换射线到局部空间，委托子物体求交，再变换法线回世界空间
// 法线规则：n_world = (M^-1)^T * n_local
bool Transform::intersect(const Ray &r, Hit &h, float tmin) {
  Matrix objectMatrix;  // 世界到局部的逆变换矩阵 M^-1
  matrix.Inverse(objectMatrix);  // 逆变换矩阵的转置

  Vec3f origin = r.getOrigin();  // 射线起点
  Vec3f direction = r.getDirection();  // 射线方向
  objectMatrix.Transform(origin);  // 变换起点
  objectMatrix.TransformDirection(direction);  // 变换方向

  Ray localRay(origin, direction);  // 变换后的射线
  Hit localHit(h);  // 变换后的交点
  if (!object->intersect(localRay, localHit, tmin))  // 委托子物体求交
    return false;  // 没有交点

  Vec3f normal = localHit.getNormal();  // 局部法线
  inverseMatrix.TransformDirection(normal);  // 变换法线
  normal.Normalize();

  h.set(localHit.getT(), localHit.getMaterial(), normal, r);  // 设置交点
  return true;
}

// 阴影射线求交
bool Transform::intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                                Material **outMaterial) {
  Matrix objectMatrix; // 世界到局部的逆变换矩阵 M^-1
  matrix.Inverse(objectMatrix); // 逆变换矩阵的转置

  Vec3f origin = r.getOrigin(); // 射线起点
  Vec3f direction = r.getDirection(); // 射线方向
  objectMatrix.Transform(origin); // 变换起点
  objectMatrix.TransformDirection(direction); // 变换方向

  Ray localRay(origin, direction); // 变换后的射线
  return object->intersectShadow(localRay, tmin, tmax, t, outMaterial); // 子物体求交
}

// OpenGL 绘制
void Transform::paint(void) const {
  glPushMatrix();  // 保存当前矩阵
  GLfloat *glMatrix = matrix.glGet();  // 获取变换矩阵
  glMultMatrixf(glMatrix);  // 应用变换矩阵
  delete[] glMatrix;  // 释放变换矩阵
  object->paint();  // 绘制子物体
  glPopMatrix();  // 恢复矩阵
}
