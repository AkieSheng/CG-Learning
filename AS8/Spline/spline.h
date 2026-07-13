#ifndef _SPLINE_H_
#define _SPLINE_H_

#include "vectors.h"

class ArgParser;
class TriangleMesh;

class Spline {

public:
  Spline() {}
  virtual ~Spline() {}

  // 可视化
  virtual void Paint(ArgParser *args) = 0;

  // Bezier / BSpline 格式输出
  virtual void OutputBezier(FILE *file) = 0;
  virtual void OutputBSpline(FILE *file) = 0;

  // 访问控制点
  virtual int getNumVertices() = 0;
  virtual Vec3f getVertex(int i) = 0;

  // 交互编辑
  virtual void moveControlPoint(int selectedPoint, float x, float y) = 0;
  virtual void addControlPoint(int selectedPoint, float x, float y) = 0;
  virtual void deleteControlPoint(int selectedPoint) = 0;

  // 三角网格输出
  virtual TriangleMesh* OutputTriangles(ArgParser *args) = 0;
};

#endif
