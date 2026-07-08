#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "image.h"
#include "scene_parser.h"
#include "hit.h"
#include "camera.h"
#include "group.h"
#include "material.h"
#include "light.h"

// 命令行参数集合
struct RayTracerArgs {
  char *input_file;    // 场景描述文件路径
  int width;           // 输出图像宽度（像素）
  int height;          // 输出图像高度（像素）
  char *output_file;   // 漫反射着色输出 TGA
  float depth_min;     // 深度可视化映射下界
  float depth_max;     // 深度可视化映射上界
  char *depth_file;    // 深度图输出 TGA
  char *normals_file;  // 法线可视化输出 TGA
  bool shade_back;     // 是否对背面着色
};

// 解析命令行参数
static void parseArgs(int argc, char *argv[], RayTracerArgs &args) {
  args.input_file = NULL;
  args.width = 100;
  args.height = 100;
  args.output_file = NULL;
  args.depth_min = 0.0f;
  args.depth_max = 1.0f;
  args.depth_file = NULL;
  args.normals_file = NULL;
  args.shade_back = false;

  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-input")) {
      i++; assert(i < argc);
      args.input_file = argv[i];
    } else if (!strcmp(argv[i], "-size")) {
      i++; assert(i < argc);
      args.width = atoi(argv[i]);
      i++; assert(i < argc);
      args.height = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-output")) {
      i++; assert(i < argc);
      args.output_file = argv[i];
    } else if (!strcmp(argv[i], "-depth")) {
      i++; assert(i < argc);
      args.depth_min = (float)atof(argv[i]);
      i++; assert(i < argc);
      args.depth_max = (float)atof(argv[i]);
      i++; assert(i < argc);
      args.depth_file = argv[i];
    } else if (!strcmp(argv[i], "-normals")) {
      i++; assert(i < argc);
      args.normals_file = argv[i];
    } else if (!strcmp(argv[i], "-shade_back")) {
      args.shade_back = true;
    } else {
      printf("Unknown command line argument: %s\n", argv[i]);
      assert(0);
    }
  }

  assert(args.input_file != NULL);
  assert(args.output_file != NULL);
}

// 颜色向量逐分量相乘（漫反射公式中的 c_light ⊙ c_object）
static Vec3f componentMultiply(const Vec3f &a, const Vec3f &b) {
  return Vec3f(a.x() * b.x(), a.y() * b.y(), a.z() * b.z());
}

// 漫反射着色：c = c_ambient ⊙ c_object + Σ clamp(L·N) * c_light ⊙ c_object
// 参考了 Phong/Blinn 模型中的 diffuse 项
static Vec3f shadeDiffuse(const Hit &hit, const Vec3f &normal,
                          const Vec3f &ambient, SceneParser &parser) {
  Vec3f objectColor = hit.getMaterial()->getDiffuseColor();
  Vec3f color = componentMultiply(ambient, objectColor);

  for (int i = 0; i < parser.getNumLights(); i++) {
    Vec3f lightDir, lightColor;
    parser.getLight(i)->getIllumination(hit.getIntersectionPoint(),
                                        lightDir, lightColor);
    float diffuse = normal.Dot3(lightDir);  // L·N，背光侧为负
    if (diffuse > 0.0f)
      color += componentMultiply(lightColor, objectColor) * diffuse;
  }
  return color;
}

// 法线可视化：将 |N| 映射为 RGB
static Vec3f shadeNormal(const Vec3f &normal) {
  return Vec3f(fabs(normal.x()), fabs(normal.y()), fabs(normal.z()));
}

// 着色前处理法线（检测背面，按 -shade_back 决定是否翻转）
static Vec3f prepareNormal(const Ray &ray, const Hit &hit, bool shade_back,
                           bool &backFacing) {
  Vec3f normal = hit.getNormal();
  backFacing = ray.getDirection().Dot3(normal) > 0.0f;
  if (shade_back && backFacing)
    normal = normal * -1.0f;
  return normal;
}

int main(int argc, char *argv[]) {
  RayTracerArgs args;
  parseArgs(argc, argv, args);

  // 解析 scene_parser 构造的相机、灯光、背景色、材质、物体组
  SceneParser parser(args.input_file);
  Camera *camera = parser.getCamera();
  Group *group = parser.getGroup();
  Vec3f background = parser.getBackgroundColor();
  Vec3f ambient = parser.getAmbientLight();

  Image image(args.width, args.height);
  Image depthImage(args.width, args.height);
  Image normalImage(args.width, args.height);

  image.SetAllPixels(background);
  depthImage.SetAllPixels(background);
  normalImage.SetAllPixels(Vec3f(0, 0, 0));  // 黑色法线图背景

  float aspect = (float)args.width / (float)args.height;  // 宽高比
  const float max_t = 1.0e30f;  // Hit 初始 t

  // 逐像素渲染
  for (int y = 0; y < args.height; y++) {
    for (int x = 0; x < args.width; x++) {
      // 像素中心映射到屏幕坐标，略加增量避免像素边界偏移
      float u = (x + 0.5f) / args.width;
      float v = (y + 0.5f) / args.height;

      // 非正方形时裁剪 UV，保持相机视场
      if (aspect > 1.0f)
        u = (u - 0.5f) / aspect + 0.5f;
      else
        v = (v - 0.5f) * aspect + 0.5f;

      Ray ray = camera->generateRay(Vec2f(u, v));
      Hit hit(max_t, NULL, Vec3f(0, 0, 0));
      bool intersected = group->intersect(ray, hit, camera->getTMin());

      if (intersected) {
        bool backFacing = false;
        Vec3f normal = prepareNormal(ray, hit, args.shade_back, backFacing);

        // 漫反射着色；未开 -shade_back 时背面着色
        if (args.shade_back || !backFacing)
          image.SetPixel(x, y, shadeDiffuse(hit, normal, ambient, parser));
        else
          image.SetPixel(x, y, Vec3f(0, 0, 0));

        // 法线图使用几何法线（没有翻转法线）
        normalImage.SetPixel(x, y, shadeNormal(hit.getNormal()));

        // 深度模式：将 t 线性映射到 [0,1] 灰度，超出范围 clamp
        float gray = (args.depth_max - hit.getT()) / (args.depth_max - args.depth_min);
        if (gray < 0.0f) gray = 0.0f;
        if (gray > 1.0f) gray = 1.0f;
        depthImage.SetPixel(x, y, Vec3f(gray, gray, gray));
      }
    }
  }

  image.SaveTGA(args.output_file);
  if (args.depth_file != NULL)
    depthImage.SaveTGA(args.depth_file);
  if (args.normals_file != NULL)
    normalImage.SaveTGA(args.normals_file);

  return 0;
}
