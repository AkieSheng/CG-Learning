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

// 阴影射线求交
bool Group::intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                            Material **outMaterial) {
  // 如果 outMaterial 为 NULL，则返回最近交点
  if (outMaterial == NULL) {
    for (int i = 0; i < numObjects; i++) {
      if (objects[i] == NULL)
        continue;
      float hitT;
      if (objects[i]->intersectShadow(r, tmin, tmax, hitT, NULL)) {
        t = hitT;
        return true;
      }
    }
    return false;
  }

  // 如果 outMaterial 不为 NULL，则返回物体材质
  bool hit = false;
  float bestT = tmax;
  Material *bestMat = NULL;
  for (int i = 0; i < numObjects; i++) {
    if (objects[i] == NULL)
      continue;
    float hitT;
    Material *hitMat = NULL;
    if (objects[i]->intersectShadow(r, tmin, bestT, hitT, &hitMat)) {
      bestT = hitT;
      bestMat = hitMat;
      hit = true;
    }
  }
  if (hit) {
    t = bestT;
    *outMaterial = bestMat;
  }
  return hit;
}

// OpenGL 绘制
void Group::paint(void) const {
  for (int i = 0; i < numObjects; i++) {
    if (objects[i] != NULL)
      objects[i]->paint();
  }
}
