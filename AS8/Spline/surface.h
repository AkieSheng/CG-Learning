#ifndef _SURFACE_H_
#define _SURFACE_H_

#include "spline.h"

class Curve;

// 曲面基层
class Surface : public Spline {

public:
  Surface() {}
  virtual ~Surface() {}

  // 编辑控制点
  void moveControlPoint(int selectedPoint, float x, float y) {}
  void addControlPoint(int selectedPoint, float x, float y) {}
  void deleteControlPoint(int selectedPoint) {}
};

#endif
