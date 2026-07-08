#ifndef _GROUP_H_
#define _GROUP_H_

#include "object3d.h"

// 场景容器
class Group : public Object3D {

public:
  Group(int numObjects);
  ~Group();

  void addObject(int index, Object3D *obj);
  virtual bool intersect(const Ray &r, Hit &h, float tmin);
  virtual void paint(void) const;

private:
  Object3D **objects;  // 子物体指针数组
  int numObjects;      // 数组容量
};

#endif
