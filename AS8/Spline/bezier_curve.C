#include "bezier_curve.h"
#include <stdio.h>
#include <assert.h>

static int BezierSegmentCount(int num_vertices) {
  assert(num_vertices >= 4);
  assert((num_vertices - 1) % 3 == 0);
  return (num_vertices - 1) / 3;  // 3n+1 个控制点组成 n 个四控制点段
}

BezierCurve::BezierCurve(int num_vertices) : Curve(num_vertices) {
  assert(num_vertices >= 4);
  assert((num_vertices - 1) % 3 == 0);
}

int BezierCurve::getNumSegments() const {
  return BezierSegmentCount(num_vertices);  // 获取分段数量
}

void BezierCurve::getSegmentControlPoints(int segment, Vec3f pts[4]) const {
  int start = segment * 3;  // 每个段有 4 个控制点
  for (int i = 0; i < 4; i++)
    pts[i] = vertices[start + i];
}

const Matrix &BezierCurve::getSegmentBasis() const {
  static Matrix B = GetBezierBasisMatrix();
  return B;
}

void BezierCurve::OutputBezier(FILE *file) {
  writeControlPoints(file, "bezier");
}

void BezierCurve::OutputBSpline(FILE *file) {
  if (num_vertices == 4) {
    Vec3f bezier[4], bspline[4];
    getSegmentControlPoints(0, bezier);
    ConvertBezierControlPointsToBSpline(bezier, bspline);
    fprintf(file, "bspline\n");
    fprintf(file, "num_vertices 4\n");
    for (int i = 0; i < 4; i++)
      fprintf(file, "%g %g %g\n", bspline[i].x(), bspline[i].y(), bspline[i].z());
  } else {
    // > 4 点互转为 B 样条输出（TODO）
    writeControlPoints(file, "bspline");
  }
}
