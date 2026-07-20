#pragma once

#include "spline.h"

struct Curve;

struct Surface : Spline {
  Surface()
  { }
  ~Surface() override {}

  auto moveControlPoint(int selectedPoint, float x, float y) -> void override {}
  auto addControlPoint(int selectedPoint, float x, float y) -> void override {}
  auto deleteControlPoint(int selectedPoint) -> void override {}
};
