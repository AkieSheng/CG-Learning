#include "filter.h"
#include "film.h"
#include <math.h>
#include <stdio.h>
#include <assert.h>

// 计算邻域内加权平均的最终颜色
Vec3f Filter::getColor(int i, int j, Film *film) {
  assert(film != NULL);
  int support = getSupportRadius();
  int width = film->getWidth();
  int height = film->getHeight();
  int numSamples = film->getNumSamples();

  Vec3f colorSum(0, 0, 0);
  float weightSum = 0.0f;

  for (int x = i - support; x <= i + support; x++) {
    for (int y = j - support; y <= j + support; y++) {
      // 边界像素裁剪
      if (x < 0 || x >= width || y < 0 || y >= height)
        continue;

      // 对所有样本按 getWeight 加权平均
      for (int n = 0; n < numSamples; n++) {
        Sample s = film->getSample(x, y, n);
        Vec2f p = s.getPosition();
        // 计算相对输出像素中心的偏移
        float fx = (x + p.x()) - (i + 0.5f);
        float fy = (y + p.y()) - (j + 0.5f);
        // 得到相对像素中心的权重
        float w = getWeight(fx, fy);
        if (w <= 0.0f)
          continue;
        // 累加颜色和权重
        colorSum += s.getColor() * w;
        weightSum += w;
      }
    }
  }

  if (weightSum < 1e-8f) {
    printf("[DEBUG] Filter::getColor(%d,%d): weightSum≈0, returning black\n",
           i, j);
    return Vec3f(0, 0, 0);
  }
  return colorSum * (1.0f / weightSum);
}
