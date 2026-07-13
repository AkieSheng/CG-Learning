#ifndef _BSPLINE_CURVE_H_
#define _BSPLINE_CURVE_H_

#include "curve.h"

// 均匀三次 B-Spline
class BSplineCurve : public Curve {

public:
  BSplineCurve(int num_vertices);
  ~BSplineCurve() {}

  void OutputBezier(FILE *file);
  void OutputBSpline(FILE *file);

protected:
  int getNumSegments() const;
  void getSegmentControlPoints(int segment, Vec3f pts[4]) const;
  const Matrix &getSegmentBasis() const;
  bool allowAddControlPoints() const { return true; }
  bool allowDeleteControlPoints() const { return true; }
};

#endif
