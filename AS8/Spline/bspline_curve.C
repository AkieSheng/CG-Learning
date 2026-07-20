#include "bspline_curve.h"
#include <cassert>
#include <cstdio>

namespace {

auto BSplineSegmentCount(int num_vertices) -> int
{
  assert(num_vertices >= 4);
  return num_vertices - 3;
}

}  // namespace

BSplineCurve::BSplineCurve(int num_vertices) : Curve(num_vertices)
{
  assert(num_vertices >= 4);
}

auto BSplineCurve::getNumSegments() const -> int
{
  return BSplineSegmentCount(num_vertices);
}

auto BSplineCurve::getSegmentControlPoints(int segment, Vec3f pts[4]) const -> void
{
  for (auto i = 0; i < 4; i++) {
    pts[i] = vertices[segment + i];
  }
}

auto BSplineCurve::getSegmentBasis() const -> Matrix const&
{
  static Matrix B = GetBSplineBasisMatrix();
  return B;
}

auto BSplineCurve::OutputBSpline(FILE* file) -> void
{
  writeControlPoints(file, "bspline");
}

auto BSplineCurve::OutputBezier(FILE* file) -> void
{
  if (num_vertices == 4)
  {
    Vec3f bspline[4];
    Vec3f bezier[4];
    getSegmentControlPoints(0, bspline);
    ConvertBSplineControlPointsToBezier(bspline, bezier);
    ::fprintf(file, "bezier\n");
    ::fprintf(file, "num_vertices 4\n");
    for (auto i = 0; i < 4; i++) {
      ::fprintf(file, "%g %g %g\n", bezier[i].x(), bezier[i].y(), bezier[i].z());
    }
  } else {
    writeControlPoints(file, "bezier");
  }
}
