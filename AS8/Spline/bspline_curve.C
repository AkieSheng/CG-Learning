#include "bspline_curve.h"
#include <stdio.h>
#include <assert.h>

static int BSplineSegmentCount(int num_vertices) {
  assert(num_vertices >= 4);
  return num_vertices - 3;  // n+3 个控制点组成 n 个滑动四控制点窗口段
}

BSplineCurve::BSplineCurve(int num_vertices) : Curve(num_vertices) {
  assert(num_vertices >= 4);
}

int BSplineCurve::getNumSegments() const {
  return BSplineSegmentCount(num_vertices);  // 获取分段数量
}

void BSplineCurve::getSegmentControlPoints(int segment, Vec3f pts[4]) const {
  for (int i = 0; i < 4; i++)  // 每个段有 4 个控制点
    pts[i] = vertices[segment + i];
}

const Matrix &BSplineCurve::getSegmentBasis() const {
  static Matrix B = GetBSplineBasisMatrix();
  return B;
}

void BSplineCurve::OutputBSpline(FILE *file) {
  writeControlPoints(file, "bspline");
}

void BSplineCurve::OutputBezier(FILE *file) {
  if (num_vertices == 4) {
    Vec3f bspline[4], bezier[4];
    getSegmentControlPoints(0, bspline);
    ConvertBSplineControlPointsToBezier(bspline, bezier);
    fprintf(file, "bezier\n");
    fprintf(file, "num_vertices 4\n");
    for (int i = 0; i < 4; i++)
      fprintf(file, "%g %g %g\n", bezier[i].x(), bezier[i].y(), bezier[i].z());
  } else {
    // > 4 点互转为贝塞尔输出（TODO）
    writeControlPoints(file, "bezier");
  }
}
