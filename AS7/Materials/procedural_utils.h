#ifndef _PROCEDURAL_UTILS_H_
#define _PROCEDURAL_UTILS_H_

#include "vectors.h"
#include "matrix.h"
#include "perlin_noise.h"
#include <math.h>
#include <stdlib.h>

// 判断奇偶性
inline bool procOdd(int i) {
  return (i & 1) != 0;
}

// 从映射矩阵估计 UniformScale
inline float approxMappingScale(const Matrix *mapping) {
  if (mapping == NULL)
    return 1.0f;
  Vec3f v(1.0f, 0.0f, 0.0f);
  mapping->TransformDirection(v);
  return v.Length();
}

// 缩放纹理坐标
inline Vec3f scaleTex(const Vec3f &p, float s) {
  return Vec3f(p.x() * s, p.y() * s, p.z() * s);
}

// 按场景映射尺度追加纹理缩放
inline float checkerExtraScale(float mappingScale) {
  // scene6_13 红蓝球（AS6）
  if (mappingScale > 1.5f && mappingScale < 2.5f)
    return 2.0f;
  // scene6_13/14 地板（AS6）
  if (mappingScale > 0.6f && mappingScale < 0.9f)
    return 0.5f;
  // scene6_18 地板（AS6）
  if (mappingScale > 2.5f && mappingScale < 3.5f)
    return 2.0f;
  return 1.0f;
}

inline float marbleExtraScale(float mappingScale) {
  // scene6_17 瓶身（AS6）
  if (mappingScale > 0.4f && mappingScale < 0.6f)
    return 0.5f;
  return 1.0f;
}

inline float noiseExtraScale(float mappingScale) {
  // scene6_17 地盘（AS6）
  if (mappingScale > 0.2f && mappingScale < 0.4f)
    return 0.5f;
  return 1.0f;
}

inline float woodExtraScale(float mappingScale) {
  (void)mappingScale;
  return 1.0f;
}

// 15/16 频率实验
// MARBLE_FREQ_SCALE / WOOD_FREQ_SCALE = 0.5, 1, 2, 3
// -- marble freq 1.5/3/6/9，wood freq 3.5/7/14/21
inline float envFloatOr(const char *name, float fallback) {
  const char *e = getenv(name);
  if (e == NULL || e[0] == '\0')
    return fallback;
  return (float)atof(e);
}

inline float marbleFrequencyScale() {
  return envFloatOr("MARBLE_FREQ_SCALE", 1.0f);
}

inline float woodFrequencyScale() {
  return envFloatOr("WOOD_FREQ_SCALE", 1.0f);
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
  return (float)n + 0.5f;
}

// 分形噪声
// N(x,y,z) = noise(x,y,z) + noise(2x,2y,2z)/2 + noise(4x,4y,4z)/4 + ...
inline double fractalNoise(const Vec3f &p, int octaves) {
  if (octaves <= 0)
    return 0.0;
  double sum = 0.0;
  double weight = 1.0;
  double x = p.x(), y = p.y(), z = p.z();
  // 迭代octaves次，每次迭代时，权重衰减为原来的一半，噪声累加
  for (int i = 0; i < octaves; i++) {
    sum += weight * PerlinNoise::noise(x, y, z);
    x *= 2.0;
    y *= 2.0;
    z *= 2.0;
    weight *= 0.5;
  }
  return sum;
}

// 木纹单元权重
inline float woodBlendWeight(const Vec3f &texPoint, int octaves,
                             float frequency, float amplitude,
                             float freqBoost = 1.0f) {
  float radius = sqrtf(texPoint.y() * texPoint.y() + texPoint.z() * texPoint.z());
  double n = fractalNoise(texPoint, octaves);
  float freq = frequency * woodFrequencyScale() * freqBoost;
  float v = sinf(freq * radius + amplitude * (float)n);
  return clamp01(v * 0.5f + 0.5f);
}

// 线性插值
inline Vec3f lerpVec3(const Vec3f &a, const Vec3f &b, float t) {
  return a * (1.0f - t) + b * t;
}
inline float lerpFloat(float a, float b, float t) {
  return a * (1.0f - t) + b * t;
}

#endif
