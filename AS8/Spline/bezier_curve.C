#include <cstdio>
#include <cassert>

#include "bezier_curve.h"

namespace {

auto BezierSegmentCount(int num_vertices) -> int {
  assert(num_vertices >= 4);
  assert((num_vertices - 1) % 3 == 0);
  return (num_vertices - 1) / 3;
}

}  // namespace

BezierCurve::BezierCurve(int num_vertices) : Curve(num_vertices) {
  assert(num_vertices >= 4);
  assert((num_vertices - 1) % 3 == 0);
}

auto BezierCurve::getNumSegments() const -> int {
  return BezierSegmentCount(num_vertices);
}

auto BezierCurve::getSegmentControlPoints(int segment, Vec3f pts[4]) const -> void {
  auto start = segment * 3;
  for (auto i = 0; i < 4; i++) {
    pts[i] = vertices[start + i];
  }
}

auto BezierCurve::getSegmentBasis() const -> Matrix const& {
  static Matrix B = GetBezierBasisMatrix();
  return B;
}

auto BezierCurve::OutputBezier(FILE* file) -> void {
  writeControlPoints(file, "bezier");
}

auto BezierCurve::OutputBSpline(FILE* file) -> void {
  if (num_vertices == 4) {
    Vec3f bezier[4];
    Vec3f bspline[4];
    getSegmentControlPoints(0, bezier);
    ConvertBezierControlPointsToBSpline(bezier, bspline);
    ::fprintf(file, "bspline\n");
    ::fprintf(file, "num_vertices 4\n");
    for (auto i = 0; i < 4; i++) {
      ::fprintf(file, "%g %g %g\n", bspline[i].x(), bspline[i].y(), bspline[i].z());
    }
  } else {
    writeControlPoints(file, "bspline");
  }
}
