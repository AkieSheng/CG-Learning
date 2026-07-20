#pragma once

#include "curve.h"

struct BSplineCurve final : Curve {
  BSplineCurve(int num_vertices);
  ~BSplineCurve() override {}

  auto OutputBezier(FILE* file) -> void override;
  auto OutputBSpline(FILE* file) -> void override;

 protected:
  auto getNumSegments() const -> int override;
  auto getSegmentControlPoints(int segment, Vec3f pts[4]) const -> void override;
  auto getSegmentBasis() const -> Matrix const& override;
  auto allowAddControlPoints() const -> bool override { return true; }
  auto allowDeleteControlPoints() const -> bool override { return true; }
};
