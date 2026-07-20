#pragma once

#include "spline.h"
#include "matrix.h"

struct ArgParser;

#define DEBUG_CURVE 0

struct Curve : Spline {
  Curve(int _num_vertices);
  ~Curve() override;

  auto getNumVertices() -> int override { return num_vertices; }
  auto getVertex(int i) -> Vec3f override;
  auto set(int i, Vec3f v) -> void;

  auto moveControlPoint(int selectedPoint, float x, float y) -> void override;
  auto addControlPoint(int selectedPoint, float x, float y) -> void override;
  auto deleteControlPoint(int selectedPoint) -> void override;

  auto Paint(ArgParser* args) -> void override;
  auto OutputTriangles(ArgParser* args) -> TriangleMesh* override;

  auto numSegments() const -> int;
  auto evaluateAlongCurve(float u) const -> Vec3f;

 protected:
  virtual auto getNumSegments() const -> int = 0;
  virtual auto getSegmentControlPoints(int segment, Vec3f pts[4]) const -> void = 0;
  virtual auto allowAddControlPoints() const -> bool = 0;
  virtual auto allowDeleteControlPoints() const -> bool = 0;

  auto evaluateSegment(int segment, float t) const -> Vec3f;
  virtual auto getSegmentBasis() const -> Matrix const& = 0;

  auto writeControlPoints(FILE* file, char const* type) const -> void;
  auto insertControlPoint(int index, Vec3f v) -> void;
  auto removeControlPoint(int index) -> void;

  int num_vertices{};
  Vec3f* vertices{};
};

auto GetBezierBasisMatrix() -> Matrix;
auto GetBSplineBasisMatrix() -> Matrix;
auto GeometryMatrixFromControlPoints(Vec3f const pts[4]) -> Matrix;
auto ControlPointsFromGeometryMatrix(Matrix const& G, Vec3f pts[4]) -> void;
auto EvaluateCubicCurve(Vec3f const pts[4], Matrix const& basis, float t) -> Vec3f;
auto ConvertBezierControlPointsToBSpline(Vec3f const bezier[4], Vec3f bspline[4]) -> void;
auto ConvertBSplineControlPointsToBezier(Vec3f const bspline[4], Vec3f bezier[4]) -> void;

#if DEBUG_CURVE
auto DebugVerifyCurveConversion(Vec3f const src[4], Vec3f const dst[4],
                                Matrix const& srcBasis, Matrix const& dstBasis,
                                char const* label) -> void;
#endif
