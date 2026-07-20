#pragma once

#include "surface.h"

struct ArgParser;

struct BezierPatch final : Surface {
  BezierPatch();
  ~BezierPatch() override;

  auto set(int i, Vec3f v) -> void;

  auto Paint(ArgParser* args) -> void override;
  auto OutputBezier(FILE* file) -> void override;
  auto OutputBSpline(FILE* file) -> void override;

  auto getNumVertices() -> int override { return 16; }
  auto getVertex(int i) -> Vec3f override;
  auto OutputTriangles(ArgParser* args) -> TriangleMesh* override;

  Vec3f control_points[16]{};
};
