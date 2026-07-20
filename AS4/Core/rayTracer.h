#pragma once

#include "vectors.h"
#include "ray.h"
#include "hit.h"


struct SceneParser;
struct Material;

struct RayTracer
{
  RayTracer(SceneParser* s, int max_bounces, float cutoff_weight, bool shadows,
            bool shade_back, bool transparent_shadows);

  auto traceRay(Ray& ray, float tmin, int bounces, float weight,
                float indexOfRefraction, Hit& hit) const -> Vec3f;

  SceneParser* parser{};
  int maxBounces{};
  float cutoffWeight{};
  bool castShadows{};
  bool shadeBack{};
  bool transparentShadows{};

  static float const RAY_EPSILON;
  static int const MAX_IOR_DEPTH;
  static float const SHADOW_ATTENUATION_SCALE;

  auto mirrorDirection(Vec3f const& normal, Vec3f const& incoming) const -> Vec3f;
  auto transmittedDirection(Vec3f const& normal, Vec3f const& incoming,
                            float index_i, float index_t,
                            Vec3f& transmitted) const -> bool;
  auto prepareNormal(Ray const& ray, Hit const& hit, bool& backFacing) const -> Vec3f;
  auto getShadowAttenuation(Vec3f const& point, Vec3f const& geomNormal,
                            Vec3f const& lightDir,
                            float distanceToLight) const -> Vec3f;
  auto computeLocalShading(Ray const& ray, Hit const& hit,
                           Vec3f const& normal) const -> Vec3f;
  auto traceRayRecursive(Ray& ray, float tmin, int bounces, float weight,
                       float indexOfRefraction, Hit& hit, float const* outsideIOR,
                       int iorDepth) const -> Vec3f;

  static auto componentMultiply(Vec3f const& a, Vec3f const& b) -> Vec3f;
  static auto hasPositive(Vec3f const& c) -> bool;
  static auto transmittanceThrough(Vec3f const& transparentColor,
                                   float distance) -> Vec3f;
  static auto isFullyBlocked(Vec3f const& attenuation) -> bool;
};
