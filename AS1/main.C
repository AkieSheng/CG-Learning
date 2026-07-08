#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "image.h"
#include "scene_parser.h"
#include "hit.h"
#include "camera.h"
#include "group.h"
#include "material.h"

int main(int argc, char *argv[]) {
  char *input_file = NULL;   // 场景描述文件路径
  int width = 100;           // 输出图像宽度（像素）
  int height = 100;          // 输出图像高度（像素）
  char *output_file = NULL;  // 颜色渲染输出 TGA
  float depth_min = 0;       // 深度可视化映射下界
  float depth_max = 1;       // 深度可视化映射上界
  char *depth_file = NULL;   // 深度图输出 TGA

  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-input")) {
      i++; assert(i < argc);
      input_file = argv[i];
    } else if (!strcmp(argv[i], "-size")) {
      i++; assert(i < argc);
      width = atoi(argv[i]);
      i++; assert(i < argc);
      height = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-output")) {
      i++; assert(i < argc);
      output_file = argv[i];
    } else if (!strcmp(argv[i], "-depth")) {
      i++; assert(i < argc);
      depth_min = (float)atof(argv[i]);
      i++; assert(i < argc);
      depth_max = (float)atof(argv[i]);
      i++; assert(i < argc);
      depth_file = argv[i];
    } else {
      printf("whoops error\n", i, argv[i]);
      assert(0);
    }
  }

  assert(input_file != NULL);
  assert(output_file != NULL);

  // 解析 scene_parser.C 构造的相机、背景色、材质、物体组
  SceneParser parser(input_file);
  Camera *camera = parser.getCamera();
  Group *group = parser.getGroup();
  Vec3f background = parser.getBackgroundColor();

  Image image(width, height);
  Image depthImage(width, height);
  image.SetAllPixels(background);
  depthImage.SetAllPixels(background);

  float aspect = (float)width / (float)height;  // 宽高比
  const float max_t = 1.0e30f;  // Hit 初始 t

  // 逐像素渲染
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      // 像素中心映射到屏幕坐标，略加增量避免像素边界偏移
      float u = (x + 0.5f) / width;
      float v = (y + 0.5f) / height;

      // 非正方形时裁剪 UV，保持相机视场
      if (aspect > 1.0f)
        u = (u - 0.5f) / aspect + 0.5f;
      else
        v = (v - 0.5f) * aspect + 0.5f;

      Ray ray = camera->generateRay(Vec2f(u, v));
      Hit hit(max_t, NULL);
      bool intersected = group->intersect(ray, hit, camera->getTMin());

      if (intersected) {  // 命中
        // 颜色模式：取最近交点材质的 diffuse 颜色
        image.SetPixel(x, y, hit.getMaterial()->getDiffuseColor());

        // 深度模式：将 t 线性映射到 [0,1] 灰度，超出范围 clamp
        // 用 (depth_max - t)，这里深度顺序可以反转，应该是看具体需要
        float gray = (depth_max - hit.getT()) / (depth_max - depth_min);
        if (gray < 0.0f) gray = 0.0f;
        if (gray > 1.0f) gray = 1.0f;
        depthImage.SetPixel(x, y, Vec3f(gray, gray, gray));
      }
    }
  }

  image.SaveTGA(output_file);
  if (depth_file != NULL)
    depthImage.SaveTGA(depth_file);

  return 0;
}
