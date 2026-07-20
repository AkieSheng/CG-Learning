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
#include "glCanvas.h"
#include "gl_options.h"
#include "gl_headers.h"

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
  bool use_gui{false};
  bool shadows{false};
  bool transparent_shadows{false};
  int max_bounces{0};
  float cutoff_weight{0.01f};
};

int tessellation_theta = 10;
int tessellation_phi = 5;
bool gouraud_shading = false;
bool specular_fix = false;

static SceneParser* globalParser = nullptr;
static RayTracer* globalRayTracer = nullptr;
static RayTracerArgs globalArgs;

static auto parseArgs(int argc, char* argv[], RayTracerArgs& args) -> void {
  args.input_file = nullptr;
  args.width = 100;
  args.height = 100;
  args.output_file = nullptr;
  args.depth_min = 0.0f;
  args.depth_max = 1.0f;
  args.depth_file = nullptr;
  args.normals_file = nullptr;
  args.shade_back = false;
  args.use_gui = false;
  args.shadows = false;
  args.transparent_shadows = false;
  args.max_bounces = 0;
  args.cutoff_weight = 0.01f;

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
    } else if (!::strcmp(argv[i], "-gui")) {
      args.use_gui = true;
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
    } else if (!::strcmp(argv[i], "-tessellation")) {
      i++;
      assert(i < argc);
      tessellation_theta = ::atoi(argv[i]);
      i++;
      assert(i < argc);
      tessellation_phi = ::atoi(argv[i]);
    } else if (!::strcmp(argv[i], "-gouraud")) {
      gouraud_shading = true;
    } else if (!::strcmp(argv[i], "-specular_fix")) {
      specular_fix = true;
    } else {
      ::printf("Unknown command line argument: %s\n", argv[i]);
      assert(0);
    }
  }

  assert(args.input_file != nullptr);
  assert(args.output_file != nullptr);

  if (args.transparent_shadows) {
    args.shadows = true;
  }
}

static auto shadeNormal(Vec3f const& normal) -> Vec3f {
  return Vec3f(::fabs(normal.x()), ::fabs(normal.y()), ::fabs(normal.z()));
}

static auto clamp01(float x) -> float {
  if (x < 0.0f) {
    return 0.0f;
  }
  if (x > 1.0f) {
    return 1.0f;
  }
  return x;
}

static auto generateCameraRay(float u, float v) -> Ray {
  auto aspect = static_cast<float>(globalArgs.width) /
                static_cast<float>(globalArgs.height);
  if (aspect > 1.0f) {
    u = (u - 0.5f) / aspect + 0.5f;
  } else {
    v = (v - 0.5f) * aspect + 0.5f;
  }
  return globalParser->getCamera()->generateRay(Vec2f(u, v));
}

static auto traceRayAtScreen(float u, float v) -> void {
  assert(globalParser != nullptr && globalRayTracer != nullptr);

  auto ray = generateCameraRay(u, v);
  Hit hit(1.0e30f, nullptr, Vec3f(0, 0, 0));
  globalRayTracer->traceRay(ray, globalParser->getCamera()->getTMin(), 0, 1.0f,
                            1.0f, hit);
}

static auto renderScene() -> void {
  assert(globalParser != nullptr && globalRayTracer != nullptr);

  auto* camera = globalParser->getCamera();
  auto background = globalParser->getBackgroundColor();

  Image image(globalArgs.width, globalArgs.height);
  Image depthImage(globalArgs.width, globalArgs.height);
  Image normalImage(globalArgs.width, globalArgs.height);

  image.SetAllPixels(background);
  depthImage.SetAllPixels(background);
  normalImage.SetAllPixels(Vec3f(0, 0, 0));

  constexpr auto max_t = 1.0e30f;

  for (auto y = 0; y < globalArgs.height; y++) {
    for (auto x = 0; x < globalArgs.width; x++) {
      auto u = (x + 0.5f) / globalArgs.width;
      auto v = (y + 0.5f) / globalArgs.height;

      auto ray = generateCameraRay(u, v);
      Hit hit(max_t, nullptr, Vec3f(0, 0, 0));
      auto color = globalRayTracer->traceRay(ray, camera->getTMin(), 0, 1.0f,
                                             1.0f, hit);

      if (hit.getMaterial() != nullptr) {
        image.SetPixel(x, y, color);
        normalImage.SetPixel(x, y, shadeNormal(hit.getNormal()));
        auto depthRange = globalArgs.depth_max - globalArgs.depth_min;
        auto gray = 0.0f;
        if (depthRange > 0.0f) {
          gray = clamp01((hit.getT() - globalArgs.depth_min) / depthRange);
        }
        depthImage.SetPixel(x, y, Vec3f(gray, gray, gray));
      }
    }
  }

  image.SaveTGA(globalArgs.output_file);
  if (globalArgs.depth_file != nullptr) {
    depthImage.SaveTGA(globalArgs.depth_file);
  }
  if (globalArgs.normals_file != nullptr) {
    normalImage.SaveTGA(globalArgs.normals_file);
  }
}

auto main(int argc, char* argv[]) -> int {
  parseArgs(argc, argv, globalArgs);

  globalParser = new SceneParser(globalArgs.input_file);
  globalRayTracer = new RayTracer(globalParser, globalArgs.max_bounces,
                                  globalArgs.cutoff_weight, globalArgs.shadows,
                                  globalArgs.shade_back,
                                  globalArgs.transparent_shadows);

  if (globalArgs.use_gui) {
    ::glutInit(&argc, argv);
    GLCanvas canvas;
    canvas.initialize(globalParser, renderScene, traceRayAtScreen);
  }

  renderScene();

  delete globalRayTracer;
  delete globalParser;
  return 0;
}
