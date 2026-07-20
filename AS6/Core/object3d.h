#pragma once

#include "material.h"
#include "ray.h"
#include "hit.h"

struct BoundingBox;
struct Grid;
struct Matrix;

struct Object3D {
  Object3D()
      : material(nullptr), bbox(nullptr), intersectionMark(0),
        hasMarkedIntersection(false),
        markedHit(1.0e30f, nullptr, Vec3f(0, 0, 0)) {}
  virtual ~Object3D();

  virtual auto intersect(Ray const& r, Hit& h, float tmin) -> bool = 0;
  virtual auto intersectShadow(Ray const& r, float tmin, float tmax, float& t,
                               Material** outMaterial) -> bool = 0;
  virtual auto paint() const -> void = 0;

  auto getIntersectionMark() const -> int { return intersectionMark; }
  auto setIntersectionMark(int mark) const -> void { intersectionMark = mark; }

  auto getHasMarkedIntersection() const -> bool { return hasMarkedIntersection; }
  auto setMarkedIntersection(Hit const& h) const -> void {
    hasMarkedIntersection = true;
    markedHit = h;
  }
  auto clearMarkedIntersection() const -> void { hasMarkedIntersection = false; }
  auto getMarkedHit() const -> Hit const& { return markedHit; }

  auto getBoundingBox() const -> BoundingBox* { return bbox; }

  virtual auto insertIntoGrid(Grid* g, Matrix* m) -> void;
  virtual auto debugPrintBoundingBox(int depth) const -> void;

protected:
  Material* material{};
  BoundingBox* bbox{};
  mutable int intersectionMark{};
  mutable bool hasMarkedIntersection{};
  mutable Hit markedHit;
};
