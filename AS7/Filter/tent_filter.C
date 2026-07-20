#include "tent_filter.h"
#include <cassert>
#include <cmath>

TentFilter::TentFilter(float radius) : radius(radius)
{
  assert(radius > 0.0f);
}

float TentFilter::getWeight(float x, float y)
{
  float d = ::sqrtf(x * x + y * y);
  if (d >= radius)
    return 0.0f;
  return 1.0f - d / radius;
}

int TentFilter::getSupportRadius()
{
  return static_cast<int>(::ceil(radius - 1e-5f));
}
