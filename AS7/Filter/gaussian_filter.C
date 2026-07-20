#include "gaussian_filter.h"
#include <cassert>
#include <cmath>

GaussianFilter::GaussianFilter(float sigma) : sigma(sigma)
{
  assert(sigma > 0.0f);
}

float GaussianFilter::getWeight(float x, float y)
{
  float d2 = x * x + y * y;
  float twoSigma = 2.0f * sigma;
  if (d2 > twoSigma * twoSigma)
    return 0.0f;
  return ::expf(-d2 / (2.0f * sigma * sigma));
}

int GaussianFilter::getSupportRadius()
{
  return static_cast<int>(::ceil(2.0f * sigma - 1e-5f));
}
