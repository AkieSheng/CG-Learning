#pragma once

#include "vectors.h"
#include "boundingbox.h"

#include <ctime>
#include <climits>

struct RayTracingStats final {
  static auto Initialize(int _width, int _height, BoundingBox* _bbox,
                         int nx, int ny, int nz) -> void;

  static auto IncrementNumNonShadowRays() -> void { num_nonshadow_rays++; }
  static auto IncrementNumShadowRays() -> void { num_shadow_rays++; }
  static auto IncrementNumIntersections() -> void { num_intersections++; }
  static auto IncrementNumGridCellsTraversed() -> void {
    num_grid_cells_traversed++;
  }

  static auto PrintStatistics() -> void;

  static int width;
  static int height;
  static BoundingBox* bbox;
  static int num_x;
  static int num_y;
  static int num_z;
  static unsigned long long start_time;
  static unsigned long long num_nonshadow_rays;
  static unsigned long long num_shadow_rays;
  static unsigned long long num_intersections;
  static unsigned long long num_grid_cells_traversed;
};
