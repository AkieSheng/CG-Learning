#ifndef _SURFACE_OF_REVOLUTION_H_
#define _SURFACE_OF_REVOLUTION_H_

#include "surface.h"

class ArgParser;
class Curve;

// 旋转曲面
class SurfaceOfRevolution : public Surface {

public:
  SurfaceOfRevolution(Curve *profile);
  ~SurfaceOfRevolution();

  void Paint(ArgParser *args);
  void OutputBezier(FILE *file);
  void OutputBSpline(FILE *file);

  // 编辑操作委托到 2D 曲线
  int getNumVertices();
  Vec3f getVertex(int i);
  void moveControlPoint(int selectedPoint, float x, float y);
  void addControlPoint(int selectedPoint, float x, float y);
  void deleteControlPoint(int selectedPoint);

  TriangleMesh* OutputTriangles(ArgParser *args);

protected:
  Curve *profile_curve;  // 截面曲线
};

#endif
