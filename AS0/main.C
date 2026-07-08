#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "image.h"
#include "ifs.h"

int main(int argc, char *argv[]) {
  char *input_file = NULL;
  int num_points = 10000;
  int num_iters = 10;
  int size = 100;
  char *output_file = NULL;

  // 解析 -input / -points / -iters / -size / -output
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-input")) {
      i++; assert(i < argc);
      input_file = argv[i];
    } else if (!strcmp(argv[i], "-points")) {
      i++; assert(i < argc);
      num_points = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-iters")) {
      i++; assert(i < argc);
      num_iters = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-size")) {
      i++; assert(i < argc);
      size = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-output")) {
      i++; assert(i < argc);
      output_file = argv[i];
    } else {
      printf("whoops error with command line argument\n");
      assert(0);
    }
  }

  assert(input_file != NULL);
  assert(output_file != NULL);

  srand((unsigned int)time(NULL));  // 每次运行不同随机序列，产生不同的 attractor

  IFS ifs;
  ifs.Input(input_file);  // 读入 IFS

  Image image(size, size);  // 创建图像
  ifs.Render(&image, num_points, num_iters);  // 渲染
  image.SaveTGA(output_file);  // 保存图像

  return 0;
}
