#include "box_filter.h"
#include <math.h>
#include <assert.h>

BoxFilter::BoxFilter(float radius) : radius(radius) {
  assert(radius > 0.0f);
}

float BoxFilter::getWeight(float x, float y) {
  if (::fabs(x) <= radius && ::fabs(y) <= radius)
    return 1.0f;
  else
    return 0.0f;
}

int BoxFilter::getSupportRadius() {
  return static_cast<int>(::ceil(radius - 1e-5f));
}
