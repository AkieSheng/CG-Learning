#pragma once

#include <cassert>
#include "vectors.h"

struct Image final {
  Image(int w, int h)
  {
    width = w;
    height = h;
    data = new Vec3f[width * height];
  }
  ~Image()
  {
    delete[] data;
  }

  auto Width() const -> int { return width; }
  auto Height() const -> int { return height; }
  auto GetPixel(int x, int y) const -> Vec3f const&
  {
    assert((x >= 0) && (x < width));
    assert((y >= 0) && (y < height));
    return data[y * width + x];
  }

  auto SetAllPixels(Vec3f const& color) -> void
  {
    for (auto i = 0; i < width * height; i++) {
      data[i] = color;
    }
  }
  auto SetPixel(int x, int y, Vec3f const& color) -> void
  {
    assert((x >= 0) && (x < width));
    assert((y >= 0) && (y < height));
    data[y * width + x] = color;
  }

  static auto LoadPPM(char const* filename) -> Image*;
  auto SavePPM(char const* filename) const -> void;
  static auto LoadTGA(char const* filename) -> Image*;
  auto SaveTGA(char const* filename) const -> void;

  static auto Compare(Image* img1, Image* img2) -> Image*;

  int width{};
  int height{};
  Vec3f* data{};
};
