#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>

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

enum SamplerType {
  SAMPLER_UNIFORM = 0,
  SAMPLER_RANDOM,
  SAMPLER_JITTERED
};

enum FilterType {
  FILTER_NONE = 0,
  FILTER_BOX,
  FILTER_TENT,
  FILTER_GAUSSIAN
};

struct RayTracerArgs {
  char* input_file{};
  int width{100};
  int height{100};
  char* output_file{};
  float depth_min{0.0f};
  float depth_max{1.0f};
  char* depth_file{};
  char* normals_file{};
  bool shade_back{false};
  bool shadows{false};
  bool transparent_shadows{false};
  int max_bounces{0};
  float cutoff_weight{0.01f};
  int grid_nx{0};
  int grid_ny{0};
  int grid_nz{0};
  bool stats{false};
  SamplerType sampler_type{SAMPLER_UNIFORM};
  int num_samples{1};
  char* samples_file{};
  int sample_zoom{1};
  FilterType filter_type{FILTER_NONE};
  float filter_param{0.0f};
  char* filter_file{};
  int filter_zoom{1};
};

bool specular_fix = false;

static SceneParser* globalParser = nullptr;
static RayTracer* globalRayTracer = nullptr;
static RayTracerArgs globalArgs;

static auto parseArgs(int argc, char* argv[], RayTracerArgs& args) -> void {
  args = RayTracerArgs{};

  for (auto i = 1; i < argc; i++) {
    if (!::strcmp(argv[i], "-input")) {
      i++;
      assert(i < argc);
      args.input_file = argv[i];
    } else if (!::strcmp(argv[i], "-size")) {
      i++;
      assert(i < argc);
      args.width = ::atoi(argv[i]);
      i++;
      assert(i < argc);
      args.height = ::atoi(argv[i]);
    } else if (!::strcmp(argv[i], "-output")) {
      i++;
      assert(i < argc);
      args.output_file = argv[i];
    } else if (!::strcmp(argv[i], "-depth")) {
      i++;
      assert(i < argc);
      args.depth_file = argv[i];
      i++;
      assert(i < argc);
      args.depth_min = static_cast<float>(::atof(argv[i]));
      i++;
      assert(i < argc);
      args.depth_max = static_cast<float>(::atof(argv[i]));
    } else if (!::strcmp(argv[i], "-normals")) {
      i++;
      assert(i < argc);
      args.normals_file = argv[i];
    } else if (!::strcmp(argv[i], "-shade_back")) {
      args.shade_back = true;
    } else if (!::strcmp(argv[i], "-shadows")) {
      args.shadows = true;
    } else if (!::strcmp(argv[i], "-transparent_shadows")) {
      args.transparent_shadows = true;
    } else if (!::strcmp(argv[i], "-bounces")) {
      i++;
      assert(i < argc);
      args.max_bounces = ::atoi(argv[i]);
    } else if (!::strcmp(argv[i], "-weight")) {
      i++;
      assert(i < argc);
      args.cutoff_weight = static_cast<float>(::atof(argv[i]));
    } else if (!::strcmp(argv[i], "-specular_fix")) {
      specular_fix = true;
    } else if (!::strcmp(argv[i], "-grid")) {
      i++;
      assert(i < argc);
      args.grid_nx = ::atoi(argv[i]);
      i++;
      assert(i < argc);
      args.grid_ny = ::atoi(argv[i]);
      i++;
      assert(i < argc);
      args.grid_nz = ::atoi(argv[i]);
    } else if (!::strcmp(argv[i], "-stats")) {
      args.stats = true;
    } else if (!::strcmp(argv[i], "-random_samples")) {
      i++;
      assert(i < argc);
      args.sampler_type = SAMPLER_RANDOM;
      args.num_samples = ::atoi(argv[i]);
      assert(args.num_samples > 0);
    } else if (!::strcmp(argv[i], "-uniform_samples")) {
      i++;
      assert(i < argc);
      args.sampler_type = SAMPLER_UNIFORM;
      args.num_samples = ::atoi(argv[i]);
      assert(args.num_samples > 0);
    } else if (!::strcmp(argv[i], "-jittered_samples")) {
      i++;
      assert(i < argc);
      args.sampler_type = SAMPLER_JITTERED;
      args.num_samples = ::atoi(argv[i]);
      assert(args.num_samples > 0);
    } else if (!::strcmp(argv[i], "-render_samples")) {
      i++;
      assert(i < argc);
      args.samples_file = argv[i];
      i++;
      assert(i < argc);
      args.sample_zoom = ::atoi(argv[i]);
      assert(args.sample_zoom > 0);
    } else if (!::strcmp(argv[i], "-box_filter")) {
      i++;
      assert(i < argc);
      args.filter_type = FILTER_BOX;
      args.filter_param = static_cast<float>(::atof(argv[i]));
      assert(args.filter_param > 0.0f);
    } else if (!::strcmp(argv[i], "-tent_filter")) {
      i++;
      assert(i < argc);
      args.filter_type = FILTER_TENT;
      args.filter_param = static_cast<float>(::atof(argv[i]));
      assert(args.filter_param > 0.0f);
    } else if (!::strcmp(argv[i], "-gaussian_filter")) {
      i++;
      assert(i < argc);
      args.filter_type = FILTER_GAUSSIAN;
      args.filter_param = static_cast<float>(::atof(argv[i]));
      assert(args.filter_param > 0.0f);
    } else if (!::strcmp(argv[i], "-render_filter")) {
      i++;
      assert(i < argc);
      args.filter_file = argv[i];
      i++;
      assert(i < argc);
      args.filter_zoom = ::atoi(argv[i]);
      assert(args.filter_zoom > 0);
    } else {
      ::printf("Unknown command line argument: %s\n", argv[i]);
      assert(0);
    }
  }

  assert(args.input_file != nullptr);
  assert(args.output_file != nullptr || args.samples_file != nullptr ||
         args.filter_file != nullptr);
  if (args.filter_file != nullptr)
    assert(args.filter_type != FILTER_NONE);
  if (args.transparent_shadows)
    args.shadows = true;
}

static auto createSampler(RayTracerArgs const& args) -> Sampler* {
  switch (args.sampler_type) {
    case SAMPLER_RANDOM:
      ::printf("[DEBUG] Sampler: RandomSampler, num_samples=%d\n",
               args.num_samples);
      return new RandomSampler(args.num_samples);
    case SAMPLER_JITTERED:
      ::printf("[DEBUG] Sampler: JitteredSampler, num_samples=%d\n",
               args.num_samples);
      return new JitteredSampler(args.num_samples);
    case SAMPLER_UNIFORM:
    default:
      ::printf("[DEBUG] Sampler: UniformSampler, num_samples=%d\n",
               args.num_samples);
      return new UniformSampler(args.num_samples);
  }
}

static auto createFilter(RayTracerArgs const& args) -> Filter* {
  switch (args.filter_type) {
    case FILTER_BOX:
      ::printf("[DEBUG] Filter: BoxFilter, radius=%.4f\n", args.filter_param);
      return new BoxFilter(args.filter_param);
    case FILTER_TENT:
      ::printf("[DEBUG] Filter: TentFilter, radius=%.4f\n", args.filter_param);
      return new TentFilter(args.filter_param);
    case FILTER_GAUSSIAN:
      ::printf("[DEBUG] Filter: GaussianFilter, sigma=%.4f\n",
               args.filter_param);
      return new GaussianFilter(args.filter_param);
    case FILTER_NONE:
    default:
      ::printf("[DEBUG] Filter: none (per-pixel sample average)\n");
      return nullptr;
  }
}

static auto shadeNormal(Vec3f const& normal) -> Vec3f {
  return Vec3f(::fabs(normal.x()), ::fabs(normal.y()), ::fabs(normal.z()));
}

static auto clamp01(float x) -> float {
  if (x < 0.0f)
    return 0.0f;
  if (x > 1.0f)
    return 1.0f;
  return x;
}

static auto generateCameraRay(float u, float v) -> Ray {
  auto aspect = static_cast<float>(globalArgs.width) /
                static_cast<float>(globalArgs.height);
  if (aspect > 1.0f)
    v = (v - 0.5f) / aspect + 0.5f;
  else
    u = (u - 0.5f) * aspect + 0.5f;
  return globalParser->getCamera()->generateRay(Vec2f(u, v));
}

static auto averagePixelSamples(Film* film, int x, int y) -> Vec3f {
  auto n = film->getNumSamples();
  Vec3f sum(0, 0, 0);
  for (auto i = 0; i < n; i++)
    sum += film->getSample(x, y, i).getColor();
  return sum * (1.0f / static_cast<float>(n));
}

static auto renderScene() -> void {
  assert(globalParser != nullptr && globalRayTracer != nullptr);

  auto* camera = globalParser->getCamera();
  auto background = globalParser->getBackgroundColor();

  auto* sampler = createSampler(globalArgs);
  auto* filter = createFilter(globalArgs);
  auto numSamples = sampler->getNumSamples();
  Film film(globalArgs.width, globalArgs.height, numSamples);

  ::printf("[DEBUG] Film: %d x %d, %d sample(s) per pixel\n",
           globalArgs.width, globalArgs.height, numSamples);
  if (filter != nullptr)
    ::printf("[DEBUG] Filter supportRadius=%d\n", filter->getSupportRadius());

  auto needSamples = (globalArgs.output_file != nullptr ||
                      globalArgs.samples_file != nullptr ||
                      globalArgs.depth_file != nullptr ||
                      globalArgs.normals_file != nullptr);

  Image* image = nullptr;
  Image* depthImage = nullptr;
  Image* normalImage = nullptr;
  if (globalArgs.output_file != nullptr) {
    image = new Image(globalArgs.width, globalArgs.height);
    image->SetAllPixels(background);
  }
  if (globalArgs.depth_file != nullptr) {
    depthImage = new Image(globalArgs.width, globalArgs.height);
    depthImage->SetAllPixels(background);
  }
  if (globalArgs.normals_file != nullptr) {
    normalImage = new Image(globalArgs.width, globalArgs.height);
    normalImage->SetAllPixels(Vec3f(0, 0, 0));
  }

  constexpr auto max_t = 1.0e30f;

  if (needSamples) {
    for (auto y = 0; y < globalArgs.height; y++) {
      if (globalArgs.height >= 50 &&
          (y % (globalArgs.height / 10) == 0 || y == globalArgs.height - 1))
        ::printf("[DEBUG] sampling row %d / %d\n", y + 1, globalArgs.height);

      for (auto x = 0; x < globalArgs.width; x++) {
        for (auto s = 0; s < numSamples; s++) {
          auto offset = sampler->getSamplePosition(s);
          auto u = (x + offset.x()) / static_cast<float>(globalArgs.width);
          auto v = (y + offset.y()) / static_cast<float>(globalArgs.height);

          auto ray = generateCameraRay(u, v);
          Hit hit(max_t, nullptr, Vec3f(0, 0, 0));
          auto color = globalRayTracer->traceRay(ray, camera->getTMin(), 0,
                                                 1.0f, 1.0f, hit);

          if (hit.getMaterial() == nullptr)
            color = background;

          film.setSample(x, y, s, offset, color);

          if (s == 0 && hit.getMaterial() != nullptr) {
            if (normalImage != nullptr)
              normalImage->SetPixel(x, y, shadeNormal(hit.getNormal()));
            if (depthImage != nullptr) {
              auto depthRange = globalArgs.depth_max - globalArgs.depth_min;
              auto gray = 0.0f;
              if (depthRange > 0.0f)
                gray = clamp01((hit.getT() - globalArgs.depth_min) / depthRange);
              depthImage->SetPixel(x, y, Vec3f(gray, gray, gray));
            }
          }
        }
      }
    }

    if (globalArgs.samples_file != nullptr || globalArgs.width <= 16) {
      ::printf("[DEBUG] Sample positions stored in Film pixel (0,0):\n");
      for (auto i = 0; i < numSamples; i++) {
        auto p = film.getSample(0, 0, i).getPosition();
        ::printf("  [%d] (%.4f, %.4f)\n", i, p.x(), p.y());
      }
    }
  } else {
    ::printf("[DEBUG] Skip TraceRay (filter visualization only)\n");
  }

  if (globalArgs.samples_file != nullptr) {
    ::printf("[DEBUG] renderSamples → %s (zoom=%d)\n",
             globalArgs.samples_file, globalArgs.sample_zoom);
    film.renderSamples(globalArgs.samples_file, globalArgs.sample_zoom);
  }

  if (globalArgs.filter_file != nullptr) {
    assert(filter != nullptr);
    ::printf("[DEBUG] Filter weights at offsets from pixel center:\n");
    const float probe[][2] = {{0.0f, 0.0f}, {0.25f, 0.0f}, {0.5f, 0.0f},
                              {1.0f, 0.0f}, {0.5f, 0.5f}, {1.5f, 0.0f}};
    for (auto k = 0; k < 6; k++) {
      auto wx = probe[k][0];
      auto wy = probe[k][1];
      ::printf("  w(%.2f,%.2f)=%.4f\n", wx, wy, filter->getWeight(wx, wy));
    }
    ::printf("[DEBUG] renderFilter → %s (zoom=%d)\n", globalArgs.filter_file,
             globalArgs.filter_zoom);
    film.renderFilter(globalArgs.filter_file, globalArgs.filter_zoom, filter);
  }

  if (image != nullptr) {
    for (auto y = 0; y < globalArgs.height; y++) {
      for (auto x = 0; x < globalArgs.width; x++) {
        auto color = (filter != nullptr) ? filter->getColor(x, y, &film)
                                         : averagePixelSamples(&film, x, y);
        image->SetPixel(x, y, color);
      }
    }
    image->SaveTGA(globalArgs.output_file);
    if (filter != nullptr)
      ::printf("[DEBUG] Wrote filtered output → %s\n", globalArgs.output_file);
    else
      ::printf("[DEBUG] Wrote output (pixel-average) → %s\n",
               globalArgs.output_file);
  }
  if (depthImage != nullptr)
    depthImage->SaveTGA(globalArgs.depth_file);
  if (normalImage != nullptr)
    normalImage->SaveTGA(globalArgs.normals_file);

  delete image;
  delete depthImage;
  delete normalImage;
  delete filter;
  delete sampler;
}

auto main(int argc, char* argv[]) -> int {
  parseArgs(argc, argv, globalArgs);
  ::srand(0);

  globalParser = new SceneParser(globalArgs.input_file);
  globalRayTracer =
      new RayTracer(globalParser, globalArgs.max_bounces, globalArgs.cutoff_weight,
                    globalArgs.shadows, globalArgs.shade_back,
                    globalArgs.transparent_shadows, globalArgs.grid_nx,
                    globalArgs.grid_ny, globalArgs.grid_nz);

  if (globalArgs.stats) {
    auto* bbox = globalParser->getGroup()->getBoundingBox();
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
