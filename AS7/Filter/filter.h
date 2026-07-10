#ifndef _FILTER_H_
#define _FILTER_H_

#include "vectors.h"

class Film;

// 滤波抽象基类
class Filter {

public:
  Filter() {}
  virtual ~Filter() {}

  virtual Vec3f getColor(int i, int j, Film *film);  // 对像素 (i,j) 在邻域内做加权平均，呈现最终颜色

  virtual float getWeight(float x, float y) = 0; // 得到相对像素中心的权重

  virtual int getSupportRadius() = 0; // 得到保守支撑半径：0 = 仅本像素，1 = 含 8 邻域
};

#endif
