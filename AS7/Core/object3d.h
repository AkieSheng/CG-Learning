#ifndef _OBJECT3D_H_
#define _OBJECT3D_H_

#include "material.h"
#include "ray.h"
#include "hit.h"

class BoundingBox;  // 轴对齐包围盒
class Grid;  // 均匀体素网格
class Matrix;  // 变换矩阵

// 物体抽象基类
class Object3D {

public:
  Object3D() : material(NULL), bbox(NULL), intersectionMark(0),
               hasMarkedIntersection(false), markedHit(1.0e30f, NULL, Vec3f(0, 0, 0)) {}
  virtual ~Object3D();

  virtual bool intersect(const Ray &r, Hit &h, float tmin) = 0;
  virtual bool intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                               Material **outMaterial) = 0;

  int getIntersectionMark() const { return intersectionMark; }
  void setIntersectionMark(int mark) const { intersectionMark = mark; }

  bool getHasMarkedIntersection() const { return hasMarkedIntersection; }
  void setMarkedIntersection(const Hit &h) const {
    hasMarkedIntersection = true;
    markedHit = h;
  }
  void clearMarkedIntersection() const { hasMarkedIntersection = false; }
  const Hit &getMarkedHit() const { return markedHit; }

  // 获取保守包围盒
  BoundingBox *getBoundingBox() const { return bbox; }

  // 将图元栅格化到体素网格
  virtual void insertIntoGrid(Grid *g, Matrix *m);

protected:
  Material *material;     // 物体材质
  BoundingBox *bbox;    // 轴对齐包围盒
  mutable int intersectionMark;  // 当前射线求交标记（grid marking）
  mutable bool hasMarkedIntersection; // 是否已标记
  mutable Hit markedHit; // 已标记交点
};

#endif
