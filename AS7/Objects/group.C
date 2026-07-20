#include "group.h"
#include <cassert>
#include "grid.h"
#include "boundingbox.h"

Group::Group(int numObjects) : numObjects(numObjects)
{
  objects = new Object3D*[numObjects];
  for (int i = 0; i < numObjects; i++)
    objects[i] = nullptr;
}

Group::~Group()
{
  for (int i = 0; i < numObjects; i++)
    delete objects[i];
  delete [] objects;
}

void Group::addObject(int index, Object3D *obj)
{
  assert(index >= 0 && index < numObjects);
  objects[index] = obj;
  if (obj == nullptr)
    return;

  BoundingBox *childBox = obj->getBoundingBox();
  if (childBox == nullptr)
    return;

  if (bbox == nullptr)
    bbox = new BoundingBox(childBox->getMin(), childBox->getMax());
  else
    bbox->Extend(childBox);
}

bool Group::intersect(Ray const&r, Hit &h, float tmin)
{
  bool hit = false;
  for (int i = 0; i < numObjects; i++) {
    if (objects[i] != nullptr && objects[i]->intersect(r, h, tmin))
      hit = true;
  }
  return hit;
}

bool Group::intersectShadow(Ray const&r, float tmin, float tmax, float &t,
                            Material **outMaterial)
{

  if (outMaterial == nullptr)
  {
    for (int i = 0; i < numObjects; i++) {
      if (objects[i] == nullptr)
        continue;
      float hitT;
      if (objects[i]->intersectShadow(r, tmin, tmax, hitT, nullptr)) {
        t = hitT;
        return true;
      }
    }
    return false;
  }

  bool hit = false;
  float bestT = tmax;
  Material *bestMat = nullptr;
  for (int i = 0; i < numObjects; i++) {
    if (objects[i] == nullptr)
      continue;
    float hitT;
    Material *hitMat = nullptr;
    if (objects[i]->intersectShadow(r, tmin, bestT, hitT, &hitMat)) {
      bestT = hitT;
      bestMat = hitMat;
      hit = true;
    }
  }
  if (hit)
  {
    t = bestT;
    *outMaterial = bestMat;
  }
  return hit;
}

void Group::insertIntoGrid(Grid *g, Matrix *m)
{
  for (int i = 0; i < numObjects; i++) {
    if (objects[i] != nullptr)
      objects[i]->insertIntoGrid(g, m);
  }
}
