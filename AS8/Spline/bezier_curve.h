#ifndef _BEZIER_CURVE_H_
#define _BEZIER_CURVE_H_

#include "curve.h"

// 三次贝塞尔曲线
class BezierCurve : public Curve {

public:
  BezierCurve(int num_vertices);
  ~BezierCurve() {}

  void OutputBezier(FILE *file);
  void OutputBSpline(FILE *file);

protected:
  int getNumSegments() const;
  void getSegmentControlPoints(int segment, Vec3f pts[4]) const;
  const Matrix &getSegmentBasis() const;
  // 不允许添加/删除控制点
  bool allowAddControlPoints() const { return false; }
  bool allowDeleteControlPoints() const { return false; }
};

#endif
