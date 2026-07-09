#ifndef _PROCEDURAL_UTILS_H_
#define _PROCEDURAL_UTILS_H_

#include "vectors.h"
#include "matrix.h"
#include "perlin_noise.h"
#include <math.h>

// 判断奇偶性
inline bool procOdd(int i) {
  return (i & 1) != 0;
}

// 将世界坐标映射到纹理空间
inline Vec3f mapToTextureSpace(const Matrix *mapping, const Vec3f &world) {
  if (mapping == NULL)
    return world;
  Vec3f p = world;
  mapping->Transform(p);
  return p;
}

// 限制值
inline float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}
inline float noiseToUnit(double n) {
  return clamp01((float)n * 0.5f + 0.5f);
}

// 分形噪声
inline double fractalNoise(const Vec3f &p, int octaves) {
  if (octaves <= 0)
    return 0.0;
  double sum = 0.0;
  double weight = 1.0;
  double total = 0.0;
  double x = p.x(), y = p.y(), z = p.z();
  // 迭代octaves次，每次迭代时，权重衰减为原来的一半，噪声累加
  for (int i = 0; i < octaves; i++) {
    sum += weight * PerlinNoise::noise(x, y, z);
    total += weight;
    x *= 2.0;
    y *= 2.0;
    z *= 2.0;
    weight *= 0.5;
  } 
  return sum / total;  // 返回平均值
}

// 线性插值
inline Vec3f lerpVec3(const Vec3f &a, const Vec3f &b, float t) {
  return a * (1.0f - t) + b * t;
}
inline float lerpFloat(float a, float b, float t) {
  return a * (1.0f - t) + b * t;
}

#endif
