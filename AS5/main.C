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
#include "rayTracer.h"
#include "glCanvas.h"
#include "gl_options.h"
#include "gl_headers.h"

// 命令行参数集合
struct RayTracerArgs {
  char *input_file;  // 输入场景文件
  int width;  // 输出图像宽度
  int height;  // 输出图像高度
  char *output_file;  // 输出图像文件
  float depth_min;  // 深度图最小深度
  float depth_max;  // 深度图最大深度
  char *depth_file;  // 深度图文件
  char *normals_file;  // 法线图文件
  bool shade_back;  // 背面着色
  bool use_gui;  // 使用 GUI
  bool shadows;  // 阴影
  bool transparent_shadows;  // 半透明阴影
  int max_bounces;  // 最大反弹次数
  float cutoff_weight;  // 截断权重
  int grid_nx;  // 网格 x 方向体素数
  int grid_ny;  // 网格 y 方向体素数
  int grid_nz;  // 网格 z 方向体素数
  bool visualize_grid;  // 是否可视化网格占用
};

// OpenGL 预览全局选项
int tessellation_theta = 10;
int tessellation_phi = 5;
bool gouraud_shading = false;
bool specular_fix = false;

// 供 GLCanvas 回调的光线追踪
static SceneParser *globalParser = NULL;
static RayTracer *globalRayTracer = NULL;
static RayTracerArgs globalArgs;

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
  args.shadows = false;
  args.transparent_shadows = false;
  args.max_bounces = 0;
  args.cutoff_weight = 0.01f;
  args.grid_nx = 0;
  args.grid_ny = 0;
  args.grid_nz = 0;
  args.visualize_grid = false;

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
    } else if (!strcmp(argv[i], "-shadows")) {
      args.shadows = true;
    } else if (!strcmp(argv[i], "-transparent_shadows")) {
      args.transparent_shadows = true;
    } else if (!strcmp(argv[i], "-bounces")) {
      i++; assert(i < argc);
      args.max_bounces = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-weight")) {
      i++; assert(i < argc);
      args.cutoff_weight = (float)atof(argv[i]);
    } else if (!strcmp(argv[i], "-tessellation")) {
      i++; assert(i < argc);
      tessellation_theta = atoi(argv[i]);
      i++; assert(i < argc);
      tessellation_phi = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-gouraud")) {
      gouraud_shading = true;
    } else if (!strcmp(argv[i], "-specular_fix")) {
      specular_fix = true;
    } else if (!strcmp(argv[i], "-grid")) {
      i++; assert(i < argc);
      args.grid_nx = atoi(argv[i]);
      i++; assert(i < argc);
      args.grid_ny = atoi(argv[i]);
      i++; assert(i < argc);
      args.grid_nz = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-visualize_grid")) {
      args.visualize_grid = true;
    } else {
      printf("Unknown command line argument: %s\n", argv[i]);
      assert(0);
    }
  }

  assert(args.input_file != NULL);
  assert(args.output_file != NULL);

  if (args.transparent_shadows)
    args.shadows = true;
}

// 法线可视化
static Vec3f shadeNormal(const Vec3f &normal) {
  return Vec3f(fabs(normal.x()), fabs(normal.y()), fabs(normal.z()));  // 将 |N| 映射为 RGB 颜色
}

// 将 x 限制在 [0,1] 范围内
static float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

// 生成相机射线
static Ray generateCameraRay(float u, float v) {
  float aspect = (float)globalArgs.width / (float)globalArgs.height;
  if (aspect > 1.0f)
    v = (v - 0.5f) / aspect + 0.5f;
  else
    u = (u - 0.5f) * aspect + 0.5f;
  return globalParser->getCamera()->generateRay(Vec2f(u, v));
}

// 对单个屏幕坐标追踪一条射线（Ray Tree 调试，按 t）
static void traceRayAtScreen(float u, float v) {
  assert(globalParser != NULL && globalRayTracer != NULL);

  Ray ray = generateCameraRay(u, v);
  Hit hit(1.0e30f, NULL, Vec3f(0, 0, 0));  // 最大深度
  globalRayTracer->traceRay(ray, globalParser->getCamera()->getTMin(),
                            0, 1.0f, 1.0f, hit);  // 追踪射线
}

// 渲染场景
static void renderScene(void) {
  assert(globalParser != NULL && globalRayTracer != NULL);

  Camera *camera = globalParser->getCamera();
  Vec3f background = globalParser->getBackgroundColor();

  Image image(globalArgs.width, globalArgs.height);
  Image depthImage(globalArgs.width, globalArgs.height);
  Image normalImage(globalArgs.width, globalArgs.height);

  image.SetAllPixels(background);
  depthImage.SetAllPixels(background);
  normalImage.SetAllPixels(Vec3f(0, 0, 0));  // 黑色法线图背景

  const float max_t = 1.0e30f;

  // 逐像素渲染
  for (int y = 0; y < globalArgs.height; y++) {
    for (int x = 0; x < globalArgs.width; x++) {
      // 像素中心映射到屏幕坐标，略加增量避免像素边界偏移
      float u = (x + 0.5f) / globalArgs.width;
      float v = (y + 0.5f) / globalArgs.height;

      Ray ray = generateCameraRay(u, v);
      Hit hit(max_t, NULL, Vec3f(0, 0, 0));  // 最大深度
      Vec3f color = globalRayTracer->traceRay(
          ray, camera->getTMin(), 0, 1.0f, 1.0f, hit);  // 追踪射线

      if (hit.getMaterial() != NULL) {
        image.SetPixel(x, y, color);
        normalImage.SetPixel(x, y, shadeNormal(hit.getNormal()));  // 法线可视化
        float depthRange = globalArgs.depth_max - globalArgs.depth_min;  // 深度范围
        float gray = 0.0f;
        if (depthRange > 0.0f)
          gray = clamp01((hit.getT() - globalArgs.depth_min) / depthRange);  // 深度可视化
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
  globalRayTracer = new RayTracer(globalParser, globalArgs.max_bounces,
                                  globalArgs.cutoff_weight,
                                  globalArgs.shadows,
                                  globalArgs.shade_back,
                                  globalArgs.transparent_shadows,
                                  globalArgs.grid_nx,
                                  globalArgs.grid_ny,
                                  globalArgs.grid_nz,
                                  globalArgs.visualize_grid);

  if (globalArgs.use_gui) {
    glutInit(&argc, argv);
    GLCanvas canvas;
    canvas.initialize(globalParser, renderScene, traceRayAtScreen,
                      globalRayTracer->getGrid(),
                      globalArgs.visualize_grid);
  }

  renderScene();

  delete globalRayTracer;
  delete globalParser;
  return 0;
}
