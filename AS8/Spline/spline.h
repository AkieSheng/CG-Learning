#pragma once

#include "vectors.h"

struct ArgParser;
struct TriangleMesh;

struct Spline {
  Spline() {}
  virtual ~Spline() {}

  virtual auto Paint(ArgParser* args) -> void = 0;
  virtual auto OutputBezier(FILE* file) -> void = 0;
  virtual auto OutputBSpline(FILE* file) -> void = 0;
  virtual auto getNumVertices() -> int = 0;
  virtual auto getVertex(int i) -> Vec3f = 0;
  virtual auto moveControlPoint(int selectedPoint, float x, float y) -> void = 0;
  virtual auto addControlPoint(int selectedPoint, float x, float y) -> void = 0;
  virtual auto deleteControlPoint(int selectedPoint) -> void = 0;
  virtual auto OutputTriangles(ArgParser* args) -> TriangleMesh* = 0;
};
