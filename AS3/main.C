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
#include "glCanvas.h"
#include "gl_options.h"
#include "gl_headers.h"

// 命令行参数集合
struct RayTracerArgs {
  char *input_file;    // 场景描述文件路径
  int width;           // 输出图像宽度（像素）
  int height;          // 输出图像高度（像素）
  char *output_file;   // Phong 着色输出 TGA
  float depth_min;     // 深度可视化映射下界
  float depth_max;     // 深度可视化映射上界
  char *depth_file;    // 深度图输出 TGA
  char *normals_file;  // 法线可视化输出 TGA
  bool shade_back;     // 是否对背面着色
  bool use_gui;        // 是否启动 OpenGL 预览
};

// OpenGL 预览全局选项
int tessellation_theta = 10;
int tessellation_phi = 5;
bool gouraud_shading = false;
bool specular_fix = false;

// 供 GLCanvas 回调的光线追踪
static SceneParser *globalParser = NULL;
static RayTracerArgs globalArgs;

// 颜色向量逐分量相乘（Phong 公式中的 c_light ⊙ c_object）
static Vec3f componentMultiply(const Vec3f &a, const Vec3f &b) {
  return Vec3f(a.x() * b.x(), a.y() * b.y(), a.z() * b.z());
}

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
  args.use_gui = false;

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
      args.depth_file = argv[i];
      i++; assert(i < argc);
      args.depth_min = (float)atof(argv[i]);
      i++; assert(i < argc);
      args.depth_max = (float)atof(argv[i]);
    } else if (!strcmp(argv[i], "-normals")) {
      i++; assert(i < argc);
      args.normals_file = argv[i];
    } else if (!strcmp(argv[i], "-shade_back")) {
      args.shade_back = true;
    } else if (!strcmp(argv[i], "-gui")) {
      args.use_gui = true;
    } else if (!strcmp(argv[i], "-tessellation")) {
      i++; assert(i < argc);
      tessellation_theta = atoi(argv[i]);
      i++; assert(i < argc);
      tessellation_phi = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-gouraud")) {
      gouraud_shading = true;
    } else if (!strcmp(argv[i], "-specular_fix")) {
      specular_fix = true;
    } else {
      printf("Unknown command line argument: %s\n", argv[i]);
      assert(0);
    }
  }

  assert(args.input_file != NULL);
  assert(args.output_file != NULL);
}

// 法线可视化
static Vec3f shadeNormal(const Vec3f &normal) {
  return Vec3f(fabs(normal.x()), fabs(normal.y()), fabs(normal.z()));  // 将 |N| 映射为 RGB 颜色
}

// 着色前处理
static Vec3f prepareNormal(const Ray &ray, const Hit &hit, bool shade_back,
                           bool &backFacing) {
  Vec3f normal = hit.getNormal();  // 几何法线
  backFacing = ray.getDirection().Dot3(normal) > 0.0f;  // 是否背光
  if (shade_back && backFacing)  // 按 -shade_back 决定是否翻转
    normal = normal * -1.0f;
  return normal;
}

// 单像素 Phong 着色：c = c_ambient ⊙ kd + Σ Material::Shade(...)
// Shade 内部实现 Blinn-Torrance 的 diffuse + specular（无 1/r^2 衰减）
static Vec3f shadePixel(const Ray &ray, const Hit &hit, bool shade_back,
                        SceneParser &parser) {
  bool backFacing = false;
  Vec3f normal = prepareNormal(ray, hit, shade_back, backFacing);  // 处理法线
  if (!shade_back && backFacing)
    return Vec3f(0, 0, 0);  // 背光区域不着色

  Vec3f objectColor = hit.getMaterial()->getDiffuseColor();
  Vec3f color = componentMultiply(parser.getAmbientLight(), objectColor);  // c_ambient ⊙ kd

  for (int i = 0; i < parser.getNumLights(); i++) {
    Vec3f lightDir, lightColor;
    float distanceToLight;  // AS3 方向光为 INFINITY，忽略距离衰减
    // 获取灯光信息
    parser.getLight(i)->getIllumination(hit.getIntersectionPoint(),
                                          lightDir, lightColor,
                                          distanceToLight);
    Hit shadedHit(hit.getT(), hit.getMaterial(), normal);  // 用翻转后的法线构造 Hit
    color += hit.getMaterial()->Shade(ray, shadedHit, lightDir, lightColor);  // 计算着色
  }
  return color;  // 返回着色结果
}

// 逐像素光线追踪主循环
static void renderScene(void) {
  assert(globalParser != NULL);

  Camera *camera = globalParser->getCamera();
  Group *group = globalParser->getGroup();
  Vec3f background = globalParser->getBackgroundColor();

  Image image(globalArgs.width, globalArgs.height);
  Image depthImage(globalArgs.width, globalArgs.height);
  Image normalImage(globalArgs.width, globalArgs.height);

  image.SetAllPixels(background);
  depthImage.SetAllPixels(background);
  normalImage.SetAllPixels(Vec3f(0, 0, 0));  // 黑色法线图背景

  float aspect = (float)globalArgs.width / (float)globalArgs.height;  // 宽高比
  const float max_t = 1.0e30f;  // Hit 初始 t

  // 逐像素渲染
  for (int y = 0; y < globalArgs.height; y++) {
    for (int x = 0; x < globalArgs.width; x++) {
      // 像素中心映射到屏幕坐标，略加增量避免像素边界偏移
      float u = (x + 0.5f) / globalArgs.width;
      float v = (y + 0.5f) / globalArgs.height;

      // 裁剪 UV 坐标
      if (aspect > 1.0f)
        u = (u - 0.5f) / aspect + 0.5f;
      else
        v = (v - 0.5f) * aspect + 0.5f;

      Ray ray = camera->generateRay(Vec2f(u, v));
      Hit hit(max_t, NULL, Vec3f(0, 0, 0));
      bool intersected = group->intersect(ray, hit, camera->getTMin());

      if (intersected) {
        image.SetPixel(x, y, shadePixel(ray, hit, globalArgs.shade_back,
                                        *globalParser));  // 着色

        normalImage.SetPixel(x, y, shadeNormal(hit.getNormal()));  // 法线图
        depthImage.SetPixel(x, y, Vec3f(gray, gray, gray));  // 深度图
      }
    }
  } 
  image.SaveTGA(globalArgs.output_file);
  if (globalArgs.depth_file != NULL)
    depthImage.SaveTGA(globalArgs.depth_file);
  if (globalArgs.normals_file != NULL)
    normalImage.SaveTGA(globalArgs.normals_file);
}

int main(int argc, char *argv[]) {
  parseArgs(argc, argv, globalArgs);

  // 解析 scene_parser 构造的相机、灯光、背景色、Phong 材质、物体组
  globalParser = new SceneParser(globalArgs.input_file);

  if (globalArgs.use_gui) {
    glutInit(&argc, argv);
    GLCanvas canvas;
    canvas.initialize(globalParser, renderScene);
  }

  renderScene();
  delete globalParser;
  return 0;
}
