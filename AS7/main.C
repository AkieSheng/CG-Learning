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
#include "raytracing_stats.h"
#include "boundingbox.h"
#include "film.h"
#include "sampler.h"
#include "random_sampler.h"
#include "uniform_sampler.h"
#include "jittered_sampler.h"
#include "filter.h"
#include "box_filter.h"
#include "tent_filter.h"
#include "gaussian_filter.h"

// 采样模式
enum SamplerType {
  SAMPLER_UNIFORM = 0,
  SAMPLER_RANDOM,
  SAMPLER_JITTERED
};

// 滤波模式
enum FilterType {
  FILTER_NONE = 0,
  FILTER_BOX,
  FILTER_TENT,
  FILTER_GAUSSIAN
};

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
  bool shadows;  // 阴影
  bool transparent_shadows;  // 半透明阴影
  int max_bounces;  // 最大反弹次数
  float cutoff_weight;  // 截断权重
  int grid_nx;  // 网格 x 方向体素数
  int grid_ny;  // 网格 y 方向体素数
  int grid_nz;  // 网格 z 方向体素数
  bool stats;  // 是否打印光线追踪统计

  // 超采样
  SamplerType sampler_type;  // 采样策略
  int num_samples;  // 每像素样本数
  char *samples_file;  // -render_samples 输出文件
  int sample_zoom;  // 采样可视化放大倍数

  // 滤波
  FilterType filter_type;  // 滤波策略
  float filter_param;  // box/tent 的 radius，或 gaussian 的 sigma
  char *filter_file;  // -render_filter 输出文件
  int filter_zoom;  // 滤波可视化放大倍数
};

bool specular_fix = false;

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
  args.shadows = false;
  args.transparent_shadows = false;
  args.max_bounces = 0;
  args.cutoff_weight = 0.01f;
  args.grid_nx = 0;
  args.grid_ny = 0;
  args.grid_nz = 0;
  args.stats = false;
  // 单中心采样
  args.sampler_type = SAMPLER_UNIFORM;
  args.num_samples = 1;
  args.samples_file = NULL;
  args.sample_zoom = 1;
  // 无滤波时对像素样本做简单的平均
  args.filter_type = FILTER_NONE;
  args.filter_param = 0.0f;
  args.filter_file = NULL;
  args.filter_zoom = 1;

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
    } else if (!strcmp(argv[i], "-specular_fix")) {
      specular_fix = true;
    } else if (!strcmp(argv[i], "-grid")) {
      i++; assert(i < argc);
      args.grid_nx = atoi(argv[i]);
      i++; assert(i < argc);
      args.grid_ny = atoi(argv[i]);
      i++; assert(i < argc);
      args.grid_nz = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-stats")) {
      args.stats = true;
    } else if (!strcmp(argv[i], "-random_samples")) {
      i++; assert(i < argc);
      args.sampler_type = SAMPLER_RANDOM;
      args.num_samples = atoi(argv[i]);
      assert(args.num_samples > 0);
    } else if (!strcmp(argv[i], "-uniform_samples")) {
      i++; assert(i < argc);
      args.sampler_type = SAMPLER_UNIFORM;
      args.num_samples = atoi(argv[i]);
      assert(args.num_samples > 0);
    } else if (!strcmp(argv[i], "-jittered_samples")) {
      i++; assert(i < argc);
      args.sampler_type = SAMPLER_JITTERED;
      args.num_samples = atoi(argv[i]);
      assert(args.num_samples > 0);
    } else if (!strcmp(argv[i], "-render_samples")) {
      i++; assert(i < argc);
      args.samples_file = argv[i];
      i++; assert(i < argc);
      args.sample_zoom = atoi(argv[i]);
      assert(args.sample_zoom > 0);
    } else if (!strcmp(argv[i], "-box_filter")) {
      i++; assert(i < argc);
      args.filter_type = FILTER_BOX;
      args.filter_param = (float)atof(argv[i]);
      assert(args.filter_param > 0.0f);
    } else if (!strcmp(argv[i], "-tent_filter")) {
      i++; assert(i < argc);
      args.filter_type = FILTER_TENT;
      args.filter_param = (float)atof(argv[i]);
      assert(args.filter_param > 0.0f);
    } else if (!strcmp(argv[i], "-gaussian_filter")) {
      i++; assert(i < argc);
      args.filter_type = FILTER_GAUSSIAN;
      args.filter_param = (float)atof(argv[i]);
      assert(args.filter_param > 0.0f);
    } else if (!strcmp(argv[i], "-render_filter")) {
      i++; assert(i < argc);
      args.filter_file = argv[i];
      i++; assert(i < argc);
      args.filter_zoom = atoi(argv[i]);
      assert(args.filter_zoom > 0);
    } else {
      printf("Unknown command line argument: %s\n", argv[i]);
      assert(0);
    }
  }

  assert(args.input_file != NULL);
  assert(args.output_file != NULL || args.samples_file != NULL ||
         args.filter_file != NULL);
  if (args.filter_file != NULL)
    assert(args.filter_type != FILTER_NONE);
  if (args.transparent_shadows)
    args.shadows = true;
}

// 创建 Sampler
static Sampler *createSampler(const RayTracerArgs &args) {
  switch (args.sampler_type) {
    case SAMPLER_RANDOM:
      printf("[DEBUG] Sampler: RandomSampler, num_samples=%d\n",
             args.num_samples);
      return new RandomSampler(args.num_samples);
    case SAMPLER_JITTERED:
      printf("[DEBUG] Sampler: JitteredSampler, num_samples=%d\n",
             args.num_samples);
      return new JitteredSampler(args.num_samples);
    case SAMPLER_UNIFORM:
    default:
      printf("[DEBUG] Sampler: UniformSampler, num_samples=%d\n",
             args.num_samples);
      return new UniformSampler(args.num_samples);
  }
}

// 创建 Filter
static Filter *createFilter(const RayTracerArgs &args) {
  switch (args.filter_type) {
    case FILTER_BOX:
      printf("[DEBUG] Filter: BoxFilter, radius=%.4f\n", args.filter_param);
      return new BoxFilter(args.filter_param);
    case FILTER_TENT:
      printf("[DEBUG] Filter: TentFilter, radius=%.4f\n", args.filter_param);
      return new TentFilter(args.filter_param);
    case FILTER_GAUSSIAN:
      printf("[DEBUG] Filter: GaussianFilter, sigma=%.4f\n", args.filter_param);
      return new GaussianFilter(args.filter_param);
    case FILTER_NONE:
    default:
      printf("[DEBUG] Filter: none (per-pixel sample average)\n");
      return NULL;
  }
}

// 法线可视化
static Vec3f shadeNormal(const Vec3f &normal) {
  return Vec3f(fabs(normal.x()), fabs(normal.y()), fabs(normal.z()));
}

// 将 x 限制在 [0,1] 范围内
static float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

// 生成相机射线（屏幕坐标 u,v ∈ [0,1]）
static Ray generateCameraRay(float u, float v) {
  float aspect = (float)globalArgs.width / (float)globalArgs.height;
  if (aspect > 1.0f)
    v = (v - 0.5f) / aspect + 0.5f;
  else
    u = (u - 0.5f) * aspect + 0.5f;
  return globalParser->getCamera()->generateRay(Vec2f(u, v));
}

// 无 Filter 时对像素样本做等权平均
static Vec3f averagePixelSamples(Film *film, int x, int y) {
  int n = film->getNumSamples();
  Vec3f sum(0, 0, 0);
  for (int i = 0; i < n; i++)
    sum += film->getSample(x, y, i).getColor();
  return sum * (1.0f / (float)n);
}

// 渲染场景
static void renderScene(void) {
  assert(globalParser != NULL && globalRayTracer != NULL);

  Camera *camera = globalParser->getCamera();
  Vec3f background = globalParser->getBackgroundColor();

  Sampler *sampler = createSampler(globalArgs);
  Filter *filter = createFilter(globalArgs);
  int numSamples = sampler->getNumSamples();
  Film film(globalArgs.width, globalArgs.height, numSamples);

  printf("[DEBUG] Film: %d x %d, %d sample(s) per pixel\n",
         globalArgs.width, globalArgs.height, numSamples);
  if (filter != NULL)
    printf("[DEBUG] Filter supportRadius=%d\n", filter->getSupportRadius());

  bool needSamples = (globalArgs.output_file != NULL ||
                      globalArgs.samples_file != NULL ||
                      globalArgs.depth_file != NULL ||
                      globalArgs.normals_file != NULL);

  Image *image = NULL;
  Image *depthImage = NULL;
  Image *normalImage = NULL;
  if (globalArgs.output_file != NULL) {
    image = new Image(globalArgs.width, globalArgs.height);
    image->SetAllPixels(background);
  }
  if (globalArgs.depth_file != NULL) {
    depthImage = new Image(globalArgs.width, globalArgs.height);
    depthImage->SetAllPixels(background);
  }
  if (globalArgs.normals_file != NULL) {
    normalImage = new Image(globalArgs.width, globalArgs.height);
    normalImage->SetAllPixels(Vec3f(0, 0, 0));
  }

  const float max_t = 1.0e30f;

  if (needSamples) {
    // 逐像素、逐样本写入 Film
    for (int y = 0; y < globalArgs.height; y++) {
      // [DEBUG] 渲染进度
      if (globalArgs.height >= 50 &&
          (y % (globalArgs.height / 10) == 0 || y == globalArgs.height - 1))
        printf("[DEBUG] sampling row %d / %d\n", y + 1, globalArgs.height);

      for (int x = 0; x < globalArgs.width; x++) {
        for (int s = 0; s < numSamples; s++) {
          Vec2f offset = sampler->getSamplePosition(s);
          // 像素内偏移 → 屏幕坐标
          float u = (x + offset.x()) / (float)globalArgs.width;
          float v = (y + offset.y()) / (float)globalArgs.height;

          Ray ray = generateCameraRay(u, v);
          Hit hit(max_t, NULL, Vec3f(0, 0, 0));
          Vec3f color = globalRayTracer->traceRay(
              ray, camera->getTMin(), 0, 1.0f, 1.0f, hit);

          // 未命中时写入背景色（与 AS6 留空背景一致）
          if (hit.getMaterial() == NULL)
            color = background;

          film.setSample(x, y, s, offset, color);

          // depth / normals：用第 0 个样本写入深度图和法线图
          if (s == 0 && hit.getMaterial() != NULL) {
            if (normalImage != NULL)
              normalImage->SetPixel(x, y, shadeNormal(hit.getNormal()));
            if (depthImage != NULL) {
              float depthRange = globalArgs.depth_max - globalArgs.depth_min;
              float gray = 0.0f;
              if (depthRange > 0.0f)
                gray = clamp01((hit.getT() - globalArgs.depth_min) / depthRange);
              depthImage->SetPixel(x, y, Vec3f(gray, gray, gray));
            }
          }
        }
      }
    }

    // [DEBUG] 在采样可视化或小分辨率时打印采样位置
    if (globalArgs.samples_file != NULL || globalArgs.width <= 16) {
      printf("[DEBUG] Sample positions stored in Film pixel (0,0):\n");
      for (int i = 0; i < numSamples; i++) {
        Vec2f p = film.getSample(0, 0, i).getPosition();
        printf("  [%d] (%.4f, %.4f)\n", i, p.x(), p.y());
      }
    }
  } else {
    printf("[DEBUG] Skip TraceRay (filter visualization only)\n");
  }

  // 采样模式可视化
  if (globalArgs.samples_file != NULL) {
    printf("[DEBUG] renderSamples → %s (zoom=%d)\n",
           globalArgs.samples_file, globalArgs.sample_zoom);
    film.renderSamples(globalArgs.samples_file, globalArgs.sample_zoom);
  }

  // 滤波权重可视化
  if (globalArgs.filter_file != NULL) {
    assert(filter != NULL);
    // [DEBUG] 打印中心附近若干点的权重
    printf("[DEBUG] Filter weights at offsets from pixel center:\n");
    const float probe[][2] = {
      {0.0f, 0.0f}, {0.25f, 0.0f}, {0.5f, 0.0f},
      {1.0f, 0.0f}, {0.5f, 0.5f}, {1.5f, 0.0f}
    };
    for (int k = 0; k < 6; k++) {
      float wx = probe[k][0], wy = probe[k][1];
      printf("  w(%.2f,%.2f)=%.4f\n", wx, wy, filter->getWeight(wx, wy));
    }
    printf("[DEBUG] renderFilter → %s (zoom=%d)\n",
           globalArgs.filter_file, globalArgs.filter_zoom);
    film.renderFilter(globalArgs.filter_file, globalArgs.filter_zoom, filter);
  }

  if (image != NULL) {
    for (int y = 0; y < globalArgs.height; y++) {
      for (int x = 0; x < globalArgs.width; x++) {
        Vec3f color = (filter != NULL)
            ? filter->getColor(x, y, &film)
            : averagePixelSamples(&film, x, y);
        image->SetPixel(x, y, color);
      }
    }
    image->SaveTGA(globalArgs.output_file);
    if (filter != NULL)
      printf("[DEBUG] Wrote filtered output → %s\n", globalArgs.output_file);
    else
      printf("[DEBUG] Wrote output (pixel-average) → %s\n",
             globalArgs.output_file);
  }
  if (depthImage != NULL)
    depthImage->SaveTGA(globalArgs.depth_file);
  if (normalImage != NULL)
    normalImage->SaveTGA(globalArgs.normals_file);

  delete image;
  delete depthImage;
  delete normalImage;
  delete filter;
  delete sampler;
}

int main(int argc, char *argv[]) {
  parseArgs(argc, argv, globalArgs);
  srand(0);  // 固定种子，使 Random/Jittered 结果可复现

  // 解析场景文件
  globalParser = new SceneParser(globalArgs.input_file);
  globalRayTracer = new RayTracer(globalParser, globalArgs.max_bounces,
                                  globalArgs.cutoff_weight,
                                  globalArgs.shadows,
                                  globalArgs.shade_back,
                                  globalArgs.transparent_shadows,
                                  globalArgs.grid_nx,
                                  globalArgs.grid_ny,
                                  globalArgs.grid_nz);

  // 初始化统计信息
  if (globalArgs.stats) {
    BoundingBox *bbox = globalParser->getGroup()->getBoundingBox();
    RayTracingStats::Initialize(globalArgs.width, globalArgs.height, bbox,
                                globalArgs.grid_nx, globalArgs.grid_ny,
                                globalArgs.grid_nz);
  }

  renderScene();

  if (globalArgs.stats)
    RayTracingStats::PrintStatistics();

  delete globalRayTracer;
  delete globalParser;
  return 0;
}
