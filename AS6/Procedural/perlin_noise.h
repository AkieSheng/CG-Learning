#pragma once

#include <cmath>

struct PerlinNoise final {
  static auto noise(double x, double y, double z) -> double {
    int X = static_cast<int>(::floor(x)) & 255;
    int Y = static_cast<int>(::floor(y)) & 255;
    int Z = static_cast<int>(::floor(z)) & 255;
    x -= ::floor(x);
    y -= ::floor(y);
    z -= ::floor(z);
    double u = fade(x);
    double v = fade(y);
    double w = fade(z);
    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;
    return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
                                     grad(p[BA], x - 1, y, z)),
                             lerp(u, grad(p[AB], x, y - 1, z),
                                     grad(p[BB], x - 1, y - 1, z))),
                lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
                                     grad(p[BA + 1], x - 1, y, z - 1)),
                             lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                                     grad(p[BB + 1], x - 1, y - 1, z - 1))));
  }

private:
  static auto fade(double t) -> double {
    return t * t * t * (t * (t * 6 - 15) + 10);
  }
  static auto lerp(double t, double a, double b) -> double {
    return a + t * (b - a);
  }
  static auto grad(int hash, double x, double y, double z) -> double {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
  }

  static int p[512];
};
