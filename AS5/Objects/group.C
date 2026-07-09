#include "group.h"
#include "grid.h"
#include "boundingbox.h"
#include <assert.h>
#include <stdio.h>

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

// 添加子物体并合并其包围盒
void Group::addObject(int index, Object3D *obj) {
  assert(index >= 0 && index < numObjects);
  objects[index] = obj;
  if (obj == NULL)
    return;

  BoundingBox *childBox = obj->getBoundingBox();
  if (childBox == NULL)
    return;

  if (bbox == NULL)
    bbox = new BoundingBox(childBox->getMin(), childBox->getMax());
  else
    bbox->Extend(childBox);
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

// 将每个子物体写入网格
void Group::insertIntoGrid(Grid *g, Matrix *m) {
  for (int i = 0; i < numObjects; i++) {
    if (objects[i] != NULL)
      objects[i]->insertIntoGrid(g, m);
  }
}

void Group::debugPrintBoundingBox(int depth) const {
  for (int i = 0; i < depth; i++)
    printf("  ");
  printf("Group: ");
  if (bbox == NULL)
    printf("NULL bounding box\n");
  else
    bbox->Print();

  for (int i = 0; i < numObjects; i++) {
    if (objects[i] != NULL)
      objects[i]->debugPrintBoundingBox(depth + 1);
  }
}
