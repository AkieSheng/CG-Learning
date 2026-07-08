#include "group.h"
#include <assert.h>

// 构造场景容器
Group::Group(int numObjects) : numObjects(numObjects) {
  objects = new Object3D*[numObjects];
  for (int i = 0; i < numObjects; i++)
    objects[i] = NULL;
}

// 析构时释放子物体
Group::~Group() {
  for (int i = 0; i < numObjects; i++)
    delete objects[i];
  delete [] objects;
}

// 添加子物体
void Group::addObject(int index, Object3D *obj) {
  assert(index >= 0 && index < numObjects);
  objects[index] = obj;
}

// 求交
bool Group::intersect(const Ray &r, Hit &h, float tmin) {
  bool hit = false;
  for (int i = 0; i < numObjects; i++) {
    if (objects[i] != NULL && objects[i]->intersect(r, h, tmin))
      hit = true;
  }
  return hit;
}
