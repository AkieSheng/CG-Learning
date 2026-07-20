#pragma once

#include <cassert>

struct Object3D;

using Object3DPtr = Object3D*;

struct Object3DVector final {
  Object3DVector()
  {
    num_objects = 0;
    size = 10;
    objects = new Object3D*[size];
    for (int i = 0; i < size; i++)
      objects[i] = nullptr;
  }

  ~Object3DVector()
  { delete[] objects; }

  auto getNumObjects() const -> int { return num_objects; }
  auto getObject(int i) const -> Object3D*
  {
    assert(i >= 0 && i < num_objects);
    assert(objects[i] != nullptr);
    return objects[i];
  }

  auto addObject(Object3D* o) -> void
  {
    assert(o != nullptr);
    if (size == num_objects)
    {
      int new_size = size * 2;
      Object3D** new_objects = new Object3D*[new_size];
      for (int i = 0; i < size; i++)
        new_objects[i] = objects[i];
      for (int i = size; i < 2 * size; i++)
        new_objects[i] = nullptr;
      delete[] objects;
      objects = new_objects;
      size = new_size;
    }
    objects[num_objects] = o;
    num_objects++;
  }

  Object3D** objects{};
  int size{};
  int num_objects{};
};
