#include "filter.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include "film.h"

Vec3f Filter::getColor(int i, int j, Film *film)
{
  assert(film != nullptr);
  int support = getSupportRadius();
  int width = film->getWidth();
  int height = film->getHeight();
  int numSamples = film->getNumSamples();

  Vec3f colorSum(0, 0, 0);
  float weightSum = 0.0f;

  for (int x = i - support; x <= i + support; x++) {
    for (int y = j - support; y <= j + support; y++) {

      if (x < 0 || x >= width || y < 0 || y >= height)
        continue;

      for (int n = 0; n < numSamples; n++) {
        Sample s = film->getSample(x, y, n);
        Vec2f p = s.getPosition();

        float fx = (x + p.x()) - (i + 0.5f);
        float fy = (y + p.y()) - (j + 0.5f);

        float w = getWeight(fx, fy);
        if (w <= 0.0f)
          continue;

        colorSum += s.getColor() * w;
        weightSum += w;
      }
    }
  }

  if (weightSum < 1e-8f)
  {
    ::printf("[DEBUG] Filter::getColor(%d,%d): weightSum≈0, returning black\n",
           i, j);
    return Vec3f(0, 0, 0);
  }
  return colorSum * (1.0f / weightSum);
}
