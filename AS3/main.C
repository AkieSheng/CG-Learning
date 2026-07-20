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

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct RayTracerArgs
{
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
};

int tessellation_theta = 10;
int tessellation_phi = 5;
bool gouraud_shading = false;
bool specular_fix = false;

static SceneParser* globalParser = nullptr;
static RayTracerArgs globalArgs;

static auto componentMultiply(Vec3f const& a, Vec3f const& b) -> Vec3f
{
  return Vec3f(a.x() * b.x(), a.y() * b.y(), a.z() * b.z());
}

static auto parseArgs(int argc, char* argv[], RayTracerArgs& args) -> void
{
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
}

static auto shadeNormal(Vec3f const& normal) -> Vec3f
{
  return Vec3f(::fabsf(normal.x()), ::fabsf(normal.y()), ::fabsf(normal.z()));
}

static auto prepareNormal(Ray const& ray, Hit const& hit, bool shade_back,
                          bool& backFacing) -> Vec3f
{
  auto normal = hit.getNormal();
  backFacing = ray.getDirection().Dot3(normal) > 0.0f;
  if (shade_back && backFacing)
    normal = normal * -1.0f;
  return normal;
}

static auto shadePixel(Ray const& ray, Hit const& hit, bool shade_back,
                       SceneParser& parser) -> Vec3f
{
  bool backFacing = false;
  auto normal = prepareNormal(ray, hit, shade_back, backFacing);
  if (!shade_back && backFacing)
    return Vec3f(0, 0, 0);

  auto objectColor = hit.getMaterial()->getDiffuseColor();
  auto color = componentMultiply(parser.getAmbientLight(), objectColor);

  for (auto i = 0; i < parser.getNumLights(); i++) {
    Vec3f lightDir;
    Vec3f lightColor;
    float distanceToLight;
    parser.getLight(i)->getIllumination(hit.getIntersectionPoint(),
                                        lightDir, lightColor,
                                        distanceToLight);
    Hit shadedHit(hit.getT(), hit.getMaterial(), normal);
    color += hit.getMaterial()->Shade(ray, shadedHit, lightDir, lightColor);
  }
  return color;
}

static auto renderScene() -> void
{
  assert(globalParser != nullptr);

  auto* camera = globalParser->getCamera();
  auto* group = globalParser->getGroup();
  auto background = globalParser->getBackgroundColor();

  auto image = Image(globalArgs.width, globalArgs.height);
  auto depthImage = Image(globalArgs.width, globalArgs.height);
  auto normalImage = Image(globalArgs.width, globalArgs.height);

  image.SetAllPixels(background);
  depthImage.SetAllPixels(background);
  normalImage.SetAllPixels(Vec3f(0, 0, 0));

  auto aspect = static_cast<float>(globalArgs.width) /
                static_cast<float>(globalArgs.height);
  constexpr auto max_t = 1.0e30f;

  for (auto y = 0; y < globalArgs.height; y++) {
    for (auto x = 0; x < globalArgs.width; x++) {
      auto u = (x + 0.5f) / globalArgs.width;
      auto v = (y + 0.5f) / globalArgs.height;

      if (aspect > 1.0f) {
        u = (u - 0.5f) / aspect + 0.5f;
      } else {
        v = (v - 0.5f) * aspect + 0.5f;
      }

      auto ray = camera->generateRay(Vec2f(u, v));
      auto hit = Hit(max_t, nullptr, Vec3f(0, 0, 0));
      auto intersected = group->intersect(ray, hit, camera->getTMin());

      if (intersected) {
        image.SetPixel(x, y, shadePixel(ray, hit, globalArgs.shade_back,
                                        *globalParser));

        normalImage.SetPixel(x, y, shadeNormal(hit.getNormal()));

        float gray = (globalArgs.depth_max - hit.getT()) /
                     (globalArgs.depth_max - globalArgs.depth_min);
        if (gray < 0) gray = 0; if (gray > 1) gray = 1;
        depthImage.SetPixel(x, y, Vec3f(gray, gray, gray));
      }
    }
  }
  image.SaveTGA(globalArgs.output_file);
  if (globalArgs.depth_file != nullptr)
    depthImage.SaveTGA(globalArgs.depth_file);
  if (globalArgs.normals_file != nullptr)
    normalImage.SaveTGA(globalArgs.normals_file);
}

auto main(int argc, char* argv[]) -> int
{
  parseArgs(argc, argv, globalArgs);

  globalParser = new SceneParser(globalArgs.input_file);

  if (globalArgs.use_gui) {
    ::glutInit(&argc, argv);
    auto canvas = GLCanvas();
    canvas.initialize(globalParser, renderScene);
  }

  renderScene();
  delete globalParser;
  return 0;
}
