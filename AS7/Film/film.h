#pragma once

#include <cassert>
#include "sample.h"

struct Filter;

struct Film final {
  Film(int _width, int _height, int _num_samples)
  {
    width = _width;
    height = _height;
    num_samples = _num_samples;
    assert(width > 0 && height > 0 && num_samples > 0);
    samples = new Sample[width * height * num_samples];
  }
  ~Film()
  { delete[] samples; }

  auto getWidth() -> int { return width; }
  auto getHeight() -> int { return height; }
  auto getNumSamples() -> int { return num_samples; }
  auto getSample(int i, int j, int n) -> Sample
  {
    return samples[getIndex(i, j, n)];
  }

  auto setSample(int x, int y, int i, Vec2f offset, Vec3f color) -> void
  {
    samples[getIndex(x, y, i)].set(offset, color);
  }

  auto renderSamples(char* samples_file, int sample_zoom) -> void;
  auto renderFilter(char* filter_file, int filter_zoom, Filter* filter) -> void;

  Film()
  { assert(0); }

  auto getIndex(int i, int j, int n) -> int
  {
    assert(i >= 0 && i < width);
    assert(j >= 0 && j < height);
    assert(n >= 0 && n < num_samples);
    return i * height * num_samples + j * num_samples + n;
  }

  int width{};
  int height{};
  int num_samples{};
  Sample* samples{};
};
