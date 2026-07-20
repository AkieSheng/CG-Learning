#pragma once

#include "surface.h"

struct ArgParser;
struct Curve;

struct SurfaceOfRevolution final : Surface {
  SurfaceOfRevolution(Curve* profile);
  ~SurfaceOfRevolution() override;

  auto Paint(ArgParser* args) -> void override;
  auto OutputBezier(FILE* file) -> void override;
  auto OutputBSpline(FILE* file) -> void override;

  auto getNumVertices() -> int override;
  auto getVertex(int i) -> Vec3f override;
  auto moveControlPoint(int selectedPoint, float x, float y) -> void override;
  auto addControlPoint(int selectedPoint, float x, float y) -> void override;
  auto deleteControlPoint(int selectedPoint) -> void override;
  auto OutputTriangles(ArgParser* args) -> TriangleMesh* override;

  Curve* profile_curve{};
};
