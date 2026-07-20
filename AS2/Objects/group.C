#include "group.h"
#include <cassert>

Group::Group(int numObjects) : numObjects(numObjects) {
  objects = new Object3D*[numObjects];
  for (auto i = 0; i < numObjects; i++) {
    objects[i] = nullptr;
  }
}

Group::~Group() {
  for (auto i = 0; i < numObjects; i++) {
    delete objects[i];
  }
  delete[] objects;
}

auto Group::addObject(int index, Object3D* obj) -> void {
  assert(index >= 0 && index < numObjects);
  objects[index] = obj;
}

auto Group::intersect(Ray const& r, Hit& h, float tmin) -> bool {
  auto hit = false;
  for (auto i = 0; i < numObjects; i++) {
    if (objects[i] != nullptr && objects[i]->intersect(r, h, tmin)) {
      hit = true;
    }
  }
  return hit;
}
