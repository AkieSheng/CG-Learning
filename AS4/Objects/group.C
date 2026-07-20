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

auto Group::intersectShadow(Ray const& r, float tmin, float tmax, float& t,
                            Material** outMaterial) -> bool {
  if (outMaterial == nullptr) {
    for (auto i = 0; i < numObjects; i++) {
      if (objects[i] == nullptr) {
        continue;
      }
      float hitT;
      if (objects[i]->intersectShadow(r, tmin, tmax, hitT, nullptr)) {
        t = hitT;
        return true;
      }
    }
    return false;
  }

  auto hit = false;
  auto bestT = tmax;
  Material* bestMat = nullptr;
  for (auto i = 0; i < numObjects; i++) {
    if (objects[i] == nullptr) {
      continue;
    }
    float hitT;
    Material* hitMat = nullptr;
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

auto Group::paint() const -> void {
  for (auto i = 0; i < numObjects; i++) {
    if (objects[i] != nullptr) {
      objects[i]->paint();
    }
  }
}
