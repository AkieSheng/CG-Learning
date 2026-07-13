#ifndef _BEZIER_PATCH_H_
#define _BEZIER_PATCH_H_

#include "surface.h"

class ArgParser;

// 4x4 Bezier Patch
class BezierPatch : public Surface {

public:
  BezierPatch();
  ~BezierPatch();

  void set(int i, Vec3f v);  // 设置控制点

  void Paint(ArgParser *args);
  void OutputBezier(FILE *file);
  void OutputBSpline(FILE *file);

  int getNumVertices() { return 16; }
  Vec3f getVertex(int i);

  TriangleMesh* OutputTriangles(ArgParser *args);

protected:
  Vec3f control_points[16];  // 控制点
};

#endif
