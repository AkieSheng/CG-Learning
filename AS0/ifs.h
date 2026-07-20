#pragma once

#include "image.h"
#include "matrix.h"

struct IFS final {
  IFS();
  ~IFS();

  auto Input(char const* filename) -> void;
  auto Render(Image* image, int num_points, int num_iters) const -> void;

  int n{};
  Matrix* transforms{};
  float* probabilities{};
};
