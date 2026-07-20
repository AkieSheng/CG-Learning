#include "rayTracer.h"
#include "scene_parser.h"
#include "group.h"
#include "material.h"
#include "light.h"
#include "gl_headers.h"
#include "rayTree.h"
#include "gl_options.h"
#include <cmath>
#include <cassert>

float const RayTracer::RAY_EPSILON = 1e-4f;
int const RayTracer::MAX_IOR_DEPTH = 16;
float const RayTracer::SHADOW_ATTENUATION_SCALE = 2.5f;

RayTracer::RayTracer(SceneParser* s, int max_bounces, float cutoff_weight,
                     bool shadows, bool shade_back, bool transparent_shadows) {
  parser = s;
  maxBounces = max_bounces;
  cutoffWeight = cutoff_weight;
  castShadows = shadows;
  shadeBack = shade_back;
  transparentShadows = transparent_shadows;
}

auto RayTracer::componentMultiply(Vec3f const& a, Vec3f const& b) -> Vec3f {
  return Vec3f(a.x() * b.x(), a.y() * b.y(), a.z() * b.z());
}

auto RayTracer::hasPositive(Vec3f const& c) -> bool {
  return c.x() > 0.0f || c.y() > 0.0f || c.z() > 0.0f;
}

auto RayTracer::isFullyBlocked(Vec3f const& attenuation) -> bool {
  return attenuation.x() <= 0.0f && attenuation.y() <= 0.0f &&
         attenuation.z() <= 0.0f;
}

auto RayTracer::transmittanceThrough(Vec3f const& transparentColor,
                                     float distance) -> Vec3f {
  auto d = distance * SHADOW_ATTENUATION_SCALE;
  return Vec3f(
      ::expf(-(1.0f - transparentColor.x()) * d),
      ::expf(-(1.0f - transparentColor.y()) * d),
      ::expf(-(1.0f - transparentColor.z()) * d));
}

auto RayTracer::mirrorDirection(Vec3f const& normal,
                                Vec3f const& incoming) const -> Vec3f {
  auto nDotI = normal.Dot3(incoming);
  return incoming - normal * (2.0f * nDotI);
}

auto RayTracer::transmittedDirection(Vec3f const& normal,
                                     Vec3f const& incoming, float index_i,
                                     float index_t, Vec3f& transmitted) const -> bool {
  auto n = normal;
  auto cosi = n.Dot3(incoming);
  if (cosi > 0.0f) {
    n = n * (-1.0f);
    cosi = -cosi;
  }
  auto eta = index_i / index_t;
  auto k = 1.0f - eta * eta * (1.0f - cosi * cosi);
  if (k < 0.0f) {
    return false;
  }
  transmitted = incoming * eta - n * (eta * cosi + ::sqrtf(k));
  transmitted.Normalize();
  return true;
}

static auto offsetRayOrigin(Vec3f const& point, Vec3f const& geomNormal,
                            Vec3f const& dir, float eps) -> Vec3f {
  auto nudge = (dir.Dot3(geomNormal) > 0.0f) ? geomNormal * eps
                                             : geomNormal * (-eps);
  return point + nudge;
}

auto RayTracer::prepareNormal(Ray const& ray, Hit const& hit,
                              bool& backFacing) const -> Vec3f {
  auto normal = hit.getNormal();
  backFacing = ray.getDirection().Dot3(normal) > 0.0f;
  if (shadeBack && backFacing) {
    normal = normal * (-1.0f);
  }
  return normal;
}

auto RayTracer::getShadowAttenuation(Vec3f const& point,
                                     Vec3f const& geomNormal,
                                     Vec3f const& lightDir,
                                     float distanceToLight) const -> Vec3f {
  auto* group = parser->getGroup();
  auto origin = offsetRayOrigin(point, geomNormal, lightDir, RAY_EPSILON);
  Ray shadowRay(origin, lightDir);

  auto const maxDist = (distanceToLight < 1.0e20f)
      ? distanceToLight - RAY_EPSILON
      : 1.0e20f;

  if (!transparentShadows) {
    float t;
    if (group->intersectShadow(shadowRay, RAY_EPSILON, maxDist, t, nullptr)) {
      RayTree::AddShadowSegment(shadowRay, 0.0f, t);
      return Vec3f(0, 0, 0);
    }
    return Vec3f(1, 1, 1);
  }

  auto attenuation = Vec3f(1, 1, 1);
  auto traveled = 0.0f;

  while (traveled < maxDist) {
    float t;
    Material* blocker = nullptr;
    if (!group->intersectShadow(shadowRay, RAY_EPSILON, maxDist - traveled, t,
                                &blocker)) {
      break;
    }

    RayTree::AddShadowSegment(shadowRay, 0.0f, t);

    auto* phong = static_cast<PhongMaterial*>(blocker);
    if (!hasPositive(phong->getTransparentColor())) {
      return Vec3f(0, 0, 0);
    }

    auto entry = shadowRay.pointAtParameter(t);
    Ray exitRay(entry + lightDir * RAY_EPSILON, lightDir);
    float tExit;
    Material* exitMat = nullptr;
    if (!group->intersectShadow(exitRay, RAY_EPSILON,
                                maxDist - traveled - t - RAY_EPSILON, tExit,
                                &exitMat)) {
      return Vec3f(0, 0, 0);
    }

    auto thickness = tExit + RAY_EPSILON;
    attenuation = componentMultiply(
        attenuation, transmittanceThrough(phong->getTransparentColor(), thickness));
    if (isFullyBlocked(attenuation)) {
      return Vec3f(0, 0, 0);
    }

    traveled += t + thickness + RAY_EPSILON;
    origin = exitRay.pointAtParameter(tExit) + lightDir * RAY_EPSILON;
    shadowRay = Ray(origin, lightDir);
  }

  return attenuation;
}

auto RayTracer::computeLocalShading(Ray const& ray, Hit const& hit,
                                    Vec3f const& normal) const -> Vec3f {
  auto* material = hit.getMaterial();
  auto color = componentMultiply(parser->getAmbientLight(),
                                 material->getDiffuseColor());

  for (auto i = 0; i < parser->getNumLights(); i++) {
    Vec3f lightDir, lightColor;
    float distanceToLight;
    parser->getLight(i)->getIllumination(hit.getIntersectionPoint(), lightDir,
                                         lightColor, distanceToLight);

    if (castShadows) {
      auto shadowAtten = getShadowAttenuation(hit.getIntersectionPoint(),
                                              hit.getNormal(), lightDir,
                                              distanceToLight);
      if (isFullyBlocked(shadowAtten)) {
        continue;
      }
      lightColor = componentMultiply(lightColor, shadowAtten);
    }

    Hit shadedHit(hit.getT(), material, normal);
    color += material->Shade(ray, shadedHit, lightDir, lightColor);
  }
  return color;
}

auto RayTracer::traceRay(Ray& ray, float tmin, int bounces, float weight,
                         float indexOfRefraction, Hit& hit) const -> Vec3f {
  float outsideIOR[MAX_IOR_DEPTH];
  return traceRayRecursive(ray, tmin, bounces, weight, indexOfRefraction, hit,
                           outsideIOR, 0);
}

auto RayTracer::traceRayRecursive(Ray& ray, float tmin, int bounces, float weight,
                                  float indexOfRefraction, Hit& hit,
                                  float const* outsideIOR, int iorDepth) const -> Vec3f {
  auto background = parser->getBackgroundColor();

  if (bounces > maxBounces || weight < cutoffWeight) {
    return background;
  }

  constexpr auto max_t = 1.0e30f;
  Hit bestHit(max_t, nullptr, Vec3f(0, 0, 0));
  auto* group = parser->getGroup();
  if (!group->intersect(ray, bestHit, tmin)) {
    return background;
  }

  hit = bestHit;
  RayTree::SetMainSegment(ray, 0.0f, hit.getT());

  auto backFacing = false;
  auto normal = prepareNormal(ray, hit, backFacing);
  if (!shadeBack && backFacing) {
    return Vec3f(0, 0, 0);
  }

  auto* phong = static_cast<PhongMaterial*>(hit.getMaterial());
  assert(phong != nullptr);

  auto color = computeLocalShading(ray, hit, normal);

  if (bounces < maxBounces && weight >= cutoffWeight) {
    auto reflectiveColor = phong->getReflectiveColor();
    if (hasPositive(reflectiveColor)) {
      auto geomNormal = hit.getNormal();
      auto reflectedDir = mirrorDirection(normal, ray.getDirection());
      reflectedDir.Normalize();
      auto origin = offsetRayOrigin(hit.getIntersectionPoint(), geomNormal,
                                    reflectedDir, RAY_EPSILON);
      Ray reflectedRay(origin, reflectedDir);
      auto newWeight = weight * reflectiveColor.Length();
      Hit reflectedHit(max_t, nullptr, Vec3f(0, 0, 0));
      float iorStack[MAX_IOR_DEPTH];
      for (auto i = 0; i < iorDepth; i++) {
        iorStack[i] = outsideIOR[i];
      }
      auto reflected = traceRayRecursive(reflectedRay, RAY_EPSILON, bounces + 1,
                                         newWeight, indexOfRefraction,
                                         reflectedHit, iorStack, iorDepth);
      RayTree::AddReflectedSegment(reflectedRay, 0.0f, reflectedHit.getT());
      color += componentMultiply(reflectiveColor, reflected);
    }

    auto transparentColor = phong->getTransparentColor();
    if (hasPositive(transparentColor)) {
      float index_i, index_t;
      auto nDotD = hit.getNormal().Dot3(ray.getDirection());
      float nextIOR;
      float iorStack[MAX_IOR_DEPTH];
      for (auto i = 0; i < iorDepth; i++) {
        iorStack[i] = outsideIOR[i];
      }
      auto nextDepth = iorDepth;

      if (nDotD < 0.0f) {
        index_i = indexOfRefraction;
        index_t = phong->getIndexOfRefraction();
        assert(iorDepth < MAX_IOR_DEPTH);
        iorStack[iorDepth] = indexOfRefraction;
        nextIOR = index_t;
        nextDepth = iorDepth + 1;
      } else {
        index_i = indexOfRefraction;
        index_t = (iorDepth > 0) ? outsideIOR[iorDepth - 1] : 1.0f;
        nextIOR = index_t;
        nextDepth = iorDepth - 1;
      }

      Vec3f transmittedDir;
      if (transmittedDirection(normal, ray.getDirection(), index_i, index_t,
                               transmittedDir)) {
        auto geomNormal = hit.getNormal();
        auto origin = offsetRayOrigin(hit.getIntersectionPoint(), geomNormal,
                                      transmittedDir, RAY_EPSILON);
        Ray transmittedRay(origin, transmittedDir);
        auto newWeight = weight * transparentColor.Length();
        Hit transmittedHit(max_t, nullptr, Vec3f(0, 0, 0));
        auto transmitted = traceRayRecursive(
            transmittedRay, RAY_EPSILON, bounces + 1, newWeight, nextIOR,
            transmittedHit, iorStack, nextDepth);
        RayTree::AddTransmittedSegment(transmittedRay, 0.0f,
                                       transmittedHit.getT());
        color += componentMultiply(transparentColor, transmitted);
      }
    }
  }

  return color;
}
