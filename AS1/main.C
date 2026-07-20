#include "image.h"
#include "scene_parser.h"
#include "hit.h"
#include "camera.h"
#include "group.h"
#include "material.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

auto main(int argc, char* argv[]) -> int
{
  char* input_file = nullptr;
  auto width = 100;
  auto height = 100;
  char* output_file = nullptr;
  auto depth_min = 0.0f;
  auto depth_max = 1.0f;
  char* depth_file = nullptr;

  for (auto i = 1; i < argc; i++) {
    if (!::strcmp(argv[i], "-input")) {
      i++;
      assert(i < argc);
      input_file = argv[i];
    } else if (!::strcmp(argv[i], "-size")) {
      i++;
      assert(i < argc);
      width = ::atoi(argv[i]);
      i++;
      assert(i < argc);
      height = ::atoi(argv[i]);
    } else if (!::strcmp(argv[i], "-output")) {
      i++;
      assert(i < argc);
      output_file = argv[i];
    } else if (!::strcmp(argv[i], "-depth")) {
      i++;
      assert(i < argc);
      depth_min = float(::atof(argv[i]));
      i++;
      assert(i < argc);
      depth_max = float(::atof(argv[i]));
      i++;
      assert(i < argc);
      depth_file = argv[i];
    } else {
      ::printf("whoops error\n");
      assert(0);
    }
  }

  assert(input_file != nullptr);
  assert(output_file != nullptr);

  auto parser = SceneParser(input_file);
  auto* camera = parser.getCamera();
  auto* group = parser.getGroup();
  auto background = parser.getBackgroundColor();

  auto image = Image(width, height);
  auto depthImage = Image(width, height);
  image.SetAllPixels(background);
  depthImage.SetAllPixels(background);

  auto aspect = float(width) / float(height);
  constexpr auto max_t = 1.0e30f;

  for (auto y = 0; y < height; y++) {
    for (auto x = 0; x < width; x++) {
      auto u = (x + 0.5f) / width;
      auto v = (y + 0.5f) / height;

      if (aspect > 1.0f) {
        u = (u - 0.5f) / aspect + 0.5f;
      } else {
        v = (v - 0.5f) * aspect + 0.5f;
      }

      auto ray = camera->generateRay(Vec2f(u, v));
      auto hit = Hit(max_t, nullptr);
      auto intersected = group->intersect(ray, hit, camera->getTMin());

      if (intersected) {
        image.SetPixel(x, y, hit.getMaterial()->getDiffuseColor());

        auto gray = (depth_max - hit.getT()) / (depth_max - depth_min);
        if (gray < 0.0f)
          gray = 0.0f;
        if (gray > 1.0f)
          gray = 1.0f;
        depthImage.SetPixel(x, y, Vec3f(gray, gray, gray));
      }
    }
  }

  image.SaveTGA(output_file);
  if (depth_file != nullptr)
    depthImage.SaveTGA(depth_file);

  return 0;
}
