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
};

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
      args.depth_min = static_cast<float>(::atof(argv[i]));
      i++;
      assert(i < argc);
      args.depth_max = static_cast<float>(::atof(argv[i]));
      i++;
      assert(i < argc);
      args.depth_file = argv[i];
    } else if (!::strcmp(argv[i], "-normals")) {
      i++;
      assert(i < argc);
      args.normals_file = argv[i];
    } else if (!::strcmp(argv[i], "-shade_back")) {
      args.shade_back = true;
    } else {
      ::printf("Unknown command line argument: %s\n", argv[i]);
      assert(0);
    }
  }

  assert(args.input_file != nullptr);
  assert(args.output_file != nullptr);
}

static auto componentMultiply(Vec3f const& a, Vec3f const& b) -> Vec3f {
  return Vec3f(a.x() * b.x(), a.y() * b.y(), a.z() * b.z());
}

static auto shadeDiffuse(Hit const& hit, Vec3f const& normal,
                         Vec3f const& ambient, SceneParser& parser) -> Vec3f {
  auto objectColor = hit.getMaterial()->getDiffuseColor();
  auto color = componentMultiply(ambient, objectColor);

  for (auto i = 0; i < parser.getNumLights(); i++) {
    Vec3f lightDir;
    Vec3f lightColor;
    parser.getLight(i)->getIllumination(hit.getIntersectionPoint(),
                                        lightDir, lightColor);
    auto diffuse = normal.Dot3(lightDir);
    if (diffuse > 0.0f) {
      color += componentMultiply(lightColor, objectColor) * diffuse;
    }
  }
  return color;
}

static auto shadeNormal(Vec3f const& normal) -> Vec3f {
  return Vec3f(::fabsf(normal.x()), ::fabsf(normal.y()), ::fabsf(normal.z()));
}

static auto prepareNormal(Ray const& ray, Hit const& hit, bool shade_back,
                          bool& backFacing) -> Vec3f {
  auto normal = hit.getNormal();
  backFacing = ray.getDirection().Dot3(normal) > 0.0f;
  if (shade_back && backFacing) {
    normal = normal * -1.0f;
  }
  return normal;
}

auto main(int argc, char* argv[]) -> int {
  RayTracerArgs args;
  parseArgs(argc, argv, args);

  auto parser = SceneParser(args.input_file);
  auto* camera = parser.getCamera();
  auto* group = parser.getGroup();
  auto background = parser.getBackgroundColor();
  auto ambient = parser.getAmbientLight();

  auto image = Image(args.width, args.height);
  auto depthImage = Image(args.width, args.height);
  auto normalImage = Image(args.width, args.height);

  image.SetAllPixels(background);
  depthImage.SetAllPixels(background);
  normalImage.SetAllPixels(Vec3f(0, 0, 0));

  auto aspect = static_cast<float>(args.width) / static_cast<float>(args.height);
  constexpr auto max_t = 1.0e30f;

  for (auto y = 0; y < args.height; y++) {
    for (auto x = 0; x < args.width; x++) {
      auto u = (x + 0.5f) / args.width;
      auto v = (y + 0.5f) / args.height;

      if (aspect > 1.0f) {
        u = (u - 0.5f) / aspect + 0.5f;
      } else {
        v = (v - 0.5f) * aspect + 0.5f;
      }

      auto ray = camera->generateRay(Vec2f(u, v));
      auto hit = Hit(max_t, nullptr, Vec3f(0, 0, 0));
      auto intersected = group->intersect(ray, hit, camera->getTMin());

      if (intersected) {
        auto backFacing = false;
        auto normal = prepareNormal(ray, hit, args.shade_back, backFacing);

        if (args.shade_back || !backFacing) {
          image.SetPixel(x, y, shadeDiffuse(hit, normal, ambient, parser));
        } else {
          image.SetPixel(x, y, Vec3f(0, 0, 0));
        }

        normalImage.SetPixel(x, y, shadeNormal(hit.getNormal()));

        auto gray = (args.depth_max - hit.getT()) / (args.depth_max - args.depth_min);
        if (gray < 0.0f) {
          gray = 0.0f;
        }
        if (gray > 1.0f) {
          gray = 1.0f;
        }
        depthImage.SetPixel(x, y, Vec3f(gray, gray, gray));
      }
    }
  }

  image.SaveTGA(args.output_file);
  if (args.depth_file != nullptr) {
    depthImage.SaveTGA(args.depth_file);
  }
  if (args.normals_file != nullptr) {
    normalImage.SaveTGA(args.normals_file);
  }

  return 0;
}
