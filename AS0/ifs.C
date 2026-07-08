#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "ifs.h"

IFS::IFS() {
  n = 0;
  transforms = NULL;
  probabilities = NULL;
}

IFS::~IFS() {
  delete [] transforms;
  delete [] probabilities;
}

// 读入 IFS
void IFS::Input(const char *filename) {
  assert(filename != NULL);

  FILE *input = fopen(filename, "r");
  assert(input != NULL);

  fscanf(input, "%d", &n);
  assert(n > 0);

  // 重复调用 Input 前释放旧数组
  delete [] transforms;
  delete [] probabilities;

  transforms = new Matrix[n];  // 分配 n 个 3x3 仿射矩阵
  probabilities = new float[n];  // 分配 n 个概率

  for (int i = 0; i < n; i++) {
    fscanf(input, "%f", &probabilities[i]);
    transforms[i].Read3x3(input);  // 读取 3x3 仿射矩阵
  }

  fclose(input);
}

// 渲染
void IFS::Render(Image *image, int num_points, int num_iters) const {
  assert(image != NULL);
  assert(n > 0);
  assert(num_points > 0);
  assert(num_iters >= 0);

  int width = image->Width();
  int height = image->Height();

  image->SetAllPixels(Vec3f(0, 0, 0));  // 黑色背景

  for (int p = 0; p < num_points; p++) {
    Vec2f v((float)rand() / RAND_MAX, (float)rand() / RAND_MAX);  // 随机起点（从单位正方形采样）
    for (int k = 0; k < num_iters; k++) {  // 迭代 num_iters 次
      float r = (float)rand() / RAND_MAX;
      float sum = 0;  // 累积概率
      int t = n - 1;
      // 按累积概率选变换（概率大的变换被选更多，收敛更快）
      for (int i = 0; i < n; i++) {
        sum += probabilities[i];
        if (r < sum) {
          t = i;
          break;
        }
      }
      transforms[t].Transform(v);  // 应用变换
    }

    // 映射像素（IFS [0,1]^2 → [0,width-1]×[0,height-1]）
    int x = (int)(v.x() * (width - 1));
    int y = (int)(v.y() * (height - 1));
    if (x >= 0 && x < width && y >= 0 && y < height) {
      image->SetPixel(x, y, Vec3f(1, 1, 1));  // 设置像素为白色
    }
  }
}
