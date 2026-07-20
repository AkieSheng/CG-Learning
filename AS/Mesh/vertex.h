#pragma once

#include "vectors.h"

struct Vertex {
  Vec3f position{};
  Vec3f normal{};
  Vec4f tangent{};
  Vec2f texCoord0{};
};
