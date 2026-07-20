#include <cstdlib>
#include <cstdio>
#include <cassert>

#include "ifs.h"

IFS::IFS() = default;

IFS::~IFS() {
  delete[] transforms;
  delete[] probabilities;
}

auto IFS::Input(char const* filename) -> void {
  assert(filename != nullptr);

  auto* input = ::fopen(filename, "r");
  assert(input != nullptr);

  ::fscanf(input, "%d", &n);
  assert(n > 0);

  delete[] transforms;
  delete[] probabilities;

  transforms = new Matrix[n];
  probabilities = new float[n];

  for (auto i = 0; i < n; i++) {
    ::fscanf(input, "%f", &probabilities[i]);
    transforms[i].Read3x3(input);
  }

  ::fclose(input);
}

auto IFS::Render(Image* image, int num_points, int num_iters) const -> void {
  assert(image != nullptr);
  assert(n > 0);
  assert(num_points > 0);
  assert(num_iters >= 0);

  auto width = image->Width();
  auto height = image->Height();

  image->SetAllPixels(Vec3f(0, 0, 0));

  for (auto p = 0; p < num_points; p++) {
    auto v = Vec2f(float(::rand()) / RAND_MAX, float(::rand()) / RAND_MAX);
    for (auto k = 0; k < num_iters; k++) {
      auto r = float(::rand()) / RAND_MAX;
      auto sum = 0.0f;
      auto t = n - 1;
      for (auto i = 0; i < n; i++) {
        sum += probabilities[i];
        if (r < sum) {
          t = i;
          break;
        }
      }
      transforms[t].Transform(v);
    }

    auto x = int(v.x() * (width - 1));
    auto y = int(v.y() * (height - 1));
    if ((x >= 0) && (x < width) && (y >= 0) && (y < height)) {
      image->SetPixel(x, y, Vec3f(1, 1, 1));
    }
  }
}
