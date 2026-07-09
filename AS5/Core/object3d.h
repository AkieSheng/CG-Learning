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
  Object3D() : material(NULL), bbox(NULL) {}
  virtual ~Object3D();

  virtual bool intersect(const Ray &r, Hit &h, float tmin) = 0;
  virtual bool intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                               Material **outMaterial) = 0;
  virtual void paint(void) const = 0;

  // 获取保守包围盒
  BoundingBox *getBoundingBox() const { return bbox; }

  // 将图元栅格化到体素网格
  virtual void insertIntoGrid(Grid *g, Matrix *m);

  // [DEBUG] 打印当前节点包围盒
  virtual void debugPrintBoundingBox(int depth) const;

protected:
  Material *material;     // 物体材质
  BoundingBox *bbox;    // 轴对齐包围盒
};

#endif
