#ifndef _IFS_H_
#define _IFS_H_

#include "image.h"
#include "matrix.h"

class IFS {  // 读取仿射变换描述，渲染 attractor

public:

  IFS();
  ~IFS();

  void Input(const char *filename);
  void Render(Image *image, int num_points, int num_iters) const;

private:

  int n;                  // 变换个数
  Matrix *transforms;     // n 个 3x3 仿射变换矩阵
  float *probabilities;   // 各变换被选中的概率

};

#endif
