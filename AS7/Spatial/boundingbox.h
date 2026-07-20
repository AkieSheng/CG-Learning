#pragma once

#include "vectors.h"
#include <cassert>
#include <cstdio>

struct BoundingBox final {
  BoundingBox(Vec3f _min, Vec3f _max) { Set(_min, _max); }
  ~BoundingBox() {}

  auto Get(Vec3f& _min, Vec3f& _max) const -> void {
    _min = min;
    _max = max;
  }
  auto getMin() const -> Vec3f { return min; }
  auto getMax() const -> Vec3f { return max; }

  auto Set(BoundingBox* bb) -> void {
    assert(bb != nullptr);
    min = bb->min;
    max = bb->max;
  }
  auto Set(Vec3f _min, Vec3f _max) -> void {
    assert(_min.x() <= _max.x() && _min.y() <= _max.y() &&
           _min.z() <= _max.z());
    min = _min;
    max = _max;
  }
  auto Extend(Vec3f const& v) -> void {
    min = Vec3f((min.x() < v.x()) ? min.x() : v.x(),
                (min.y() < v.y()) ? min.y() : v.y(),
                (min.z() < v.z()) ? min.z() : v.z());
    max = Vec3f((max.x() > v.x()) ? max.x() : v.x(),
                (max.y() > v.y()) ? max.y() : v.y(),
                (max.z() > v.z()) ? max.z() : v.z());
  }
  auto Extend(BoundingBox* bb) -> void {
    assert(bb != nullptr);
    Extend(bb->min);
    Extend(bb->max);
  }

  auto Print() const -> void {
    ::printf("%f %f %f  -> %f %f %f\n", min.x(), min.y(), min.z(), max.x(),
             max.y(), max.z());
  }

  BoundingBox() = delete;

  Vec3f min{};
  Vec3f max{};
};
