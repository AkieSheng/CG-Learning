#pragma once

#include <cassert>
#include "vectors.h"

struct Sample final {
  Sample()
  {
    position = Vec2f(0.5, 0.5);
    color = Vec3f(0, 0, 0);
  }
  ~Sample()
  { }

  auto getPosition() -> Vec2f { return position; }
  auto getColor() -> Vec3f { return color; }

  auto set(Vec2f p, Vec3f c) -> void
  {
    assert(p.x() >= 0 && p.x() <= 1);
    assert(p.y() >= 0 && p.y() <= 1);
    position = p;
    color = c;
  }

  Vec2f position{};
  Vec3f color{};
};
