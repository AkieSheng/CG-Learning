#include "rayTracer.h"

#include "scene_parser.h"
#include "group.h"
#include "material.h"
#include "light.h"
#include "gl_headers.h"
#include "rayTree.h"
#include "gl_options.h"
#include "grid.h"
#include "boundingbox.h"

#include <cmath>
#include <cassert>
#include <cstdio>

const float RayTracer::RAY_EPSILON = 1e-4f;
const int RayTracer::MAX_IOR_DEPTH = 16;
const float RayTracer::SHADOW_ATTENUATION_SCALE = 2.5f;

RayTracer::RayTracer(SceneParser *s, int max_bounces, float cutoff_weight,
                     bool shadows, bool shade_back, bool transparent_shadows,
                     int grid_nx, int grid_ny, int grid_nz, bool visualize_grid) {
  parser = s;
  maxBounces = max_bounces;
  cutoffWeight = cutoff_weight;
  castShadows = shadows;
  shadeBack = shade_back;
  transparentShadows = transparent_shadows;
  grid = nullptr;
  visualizeGrid = visualize_grid;

  if (grid_nx <= 0 || grid_ny <= 0 || grid_nz <= 0)
    return;

  Group *sceneGroup = parser->getGroup();
  BoundingBox *sceneBounds = sceneGroup->getBoundingBox();
  if (sceneBounds == nullptr)
    return;

  ::printf("=== Bounding Box Debug ===\n");
  ::printf("Scene bounding box: ");
  sceneBounds->Print();
  sceneGroup->debugPrintBoundingBox(0);

  grid = new Grid(sceneBounds, grid_nx, grid_ny, grid_nz);
  sceneGroup->insertIntoGrid(grid, nullptr);

  ::printf("=== Grid Rasterization Debug ===\n");
  grid->printOccupancy();
}

RayTracer::~RayTracer() {
  delete grid;
}

auto RayTracer::componentMultiply(Vec3f const&a, Vec3f const&b) -> Vec3f {
  return Vec3f(a.x() * b.x(), a.y() * b.y(), a.z() * b.z());
}

auto RayTracer::hasPositive(Vec3f const&c) -> bool {
  return c.x() > 0.0f || c.y() > 0.0f || c.z() > 0.0f;
}

auto RayTracer::isFullyBlocked(Vec3f const&attenuation) -> bool {
  return attenuation.x() <= 0.0f && attenuation.y() <= 0.0f &&
         attenuation.z() <= 0.0f;
}

auto RayTracer::transmittanceThrough(Vec3f const&transparentColor,
                                      float distance) -> Vec3f {
  float d = distance * SHADOW_ATTENUATION_SCALE;
  return Vec3f(
      ::expf(-(1.0f - transparentColor.x()) * d),
      ::expf(-(1.0f - transparentColor.y()) * d),
      ::expf(-(1.0f - transparentColor.z()) * d));
}

auto RayTracer::mirrorDirection(Vec3f const&normal,
                                 Vec3f const&incoming) const -> Vec3f {
  float nDotI = normal.Dot3(incoming);
  return incoming - normal * (2.0f * nDotI);
}

auto RayTracer::transmittedDirection(Vec3f const&normal,
                                     Vec3f const&incoming,
                                     float index_i, float index_t,
                                     Vec3f &transmitted) const -> bool {
  Vec3f n = normal;
  float cosi = n.Dot3(incoming);
  if (cosi > 0.0f) {
    n = n * (-1.0f);
    cosi = -cosi;
  }
  float eta = index_i / index_t;
  float k = 1.0f - eta * eta * (1.0f - cosi * cosi);
  if (k < 0.0f)
    return false;
  transmitted = incoming * eta - n * (eta * cosi + ::sqrtf(k));
  transmitted.Normalize();
  return true;
}

static auto offsetRayOrigin(Vec3f const&point, Vec3f const&geomNormal,
                             Vec3f const&dir, float eps) -> Vec3f {
  Vec3f nudge = (dir.Dot3(geomNormal) > 0.0f) ? geomNormal * eps
                                              : geomNormal * (-eps);
  return point + nudge;
}

auto RayTracer::prepareNormal(Ray const&ray, Hit const&hit,
                               bool &backFacing) const -> Vec3f {
  Vec3f normal = hit.getNormal();
  backFacing = ray.getDirection().Dot3(normal) > 0.0f;
  if (shadeBack && backFacing)
    normal = normal * (-1.0f);
  return normal;
}

auto RayTracer::getShadowAttenuation(Vec3f const&point,
                                      Vec3f const&geomNormal,
                                      Vec3f const&lightDir,
                                      float distanceToLight) const -> Vec3f {
  Group *group = parser->getGroup();
  Vec3f origin = offsetRayOrigin(point, geomNormal, lightDir, RAY_EPSILON);
  Ray shadowRay(origin, lightDir);

  const float maxDist = (distanceToLight < 1.0e20f)
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

  Vec3f attenuation(1, 1, 1);
  float traveled = 0.0f;

  while (traveled < maxDist) {
    float t;
    Material *blocker = nullptr;
    if (!group->intersectShadow(shadowRay, RAY_EPSILON, maxDist - traveled, t, &blocker))
      break;

    RayTree::AddShadowSegment(shadowRay, 0.0f, t);

    PhongMaterial* phong = static_cast<PhongMaterial*>(blocker);
    if (!hasPositive(phong->getTransparentColor()))
      return Vec3f(0, 0, 0);

    Vec3f entry = shadowRay.pointAtParameter(t);
    Ray exitRay(entry + lightDir * RAY_EPSILON, lightDir);
    float tExit;
    Material *exitMat = nullptr;
    if (!group->intersectShadow(exitRay, RAY_EPSILON, maxDist - traveled - t - RAY_EPSILON, tExit, &exitMat))
      return Vec3f(0, 0, 0);

    float thickness = tExit + RAY_EPSILON;
    attenuation = componentMultiply(
        attenuation, transmittanceThrough(phong->getTransparentColor(),
                                            thickness));
    if (isFullyBlocked(attenuation))
      return Vec3f(0, 0, 0);

    traveled += t + thickness + RAY_EPSILON;
    origin = exitRay.pointAtParameter(tExit) + lightDir * RAY_EPSILON;
    shadowRay = Ray(origin, lightDir);
  }

  return attenuation;
}

auto RayTracer::computeLocalShading(Ray const&ray, Hit const&hit,
                                     Vec3f const&normal) const -> Vec3f {
  Material *material = hit.getMaterial();
  Vec3f color = componentMultiply(parser->getAmbientLight(), material->getDiffuseColor());

  for (int i = 0; i < parser->getNumLights(); i++) {
    Vec3f lightDir, lightColor;
    float distanceToLight;
    parser->getLight(i)->getIllumination(hit.getIntersectionPoint(), lightDir, lightColor, distanceToLight);

    if (castShadows) {
      Vec3f shadowAtten = getShadowAttenuation(hit.getIntersectionPoint(),
                                               hit.getNormal(), lightDir,
                                               distanceToLight);
      if (isFullyBlocked(shadowAtten))
        continue;
      lightColor = componentMultiply(lightColor, shadowAtten);
    }

    Hit shadedHit(hit.getT(), material, normal);
    color += material->Shade(ray, shadedHit, lightDir, lightColor);
  }
  return color;
}

auto RayTracer::traceRay(Ray &ray, float tmin, int bounces, float weight,
                          float indexOfRefraction, Hit &hit) const -> Vec3f {
  float outsideIOR[MAX_IOR_DEPTH];
  return traceRayRecursive(ray, tmin, bounces, weight, indexOfRefraction, hit,
                           outsideIOR, 0);
}

auto RayTracer::traceRayRecursive(Ray &ray, float tmin, int bounces,
                                   float weight, float indexOfRefraction,
                                   Hit &hit, float const*outsideIOR,
                                   int iorDepth) const -> Vec3f {
  Vec3f background = parser->getBackgroundColor();

  if (bounces > maxBounces || weight < cutoffWeight)
    return background;

  const float max_t = 1.0e30f;
  Hit bestHit(max_t, nullptr, Vec3f(0, 0, 0));
  bool hitSomething = false;

  if (visualizeGrid && grid != nullptr) {
    hitSomething = grid->intersect(ray, bestHit, tmin);
  } else {
    Group *group = parser->getGroup();
    hitSomething = group->intersect(ray, bestHit, tmin);
  }

  if (!hitSomething)
    return background;

  hit = bestHit;
  RayTree::SetMainSegment(ray, 0.0f, hit.getT());

  bool backFacing = false;
  Vec3f normal = prepareNormal(ray, hit, backFacing);
  if (!shadeBack && backFacing)
    return Vec3f(0, 0, 0);

  PhongMaterial* phong = static_cast<PhongMaterial*>(hit.getMaterial());
  assert(phong != nullptr);

  Vec3f color = computeLocalShading(ray, hit, normal);

  if (visualizeGrid && grid != nullptr)
    return color;

  if (bounces < maxBounces && weight >= cutoffWeight) {
    Vec3f reflectiveColor = phong->getReflectiveColor();
    if (hasPositive(reflectiveColor)) {
      Vec3f geomNormal = hit.getNormal();
      Vec3f reflectedDir = mirrorDirection(normal, ray.getDirection());
      reflectedDir.Normalize();
      Vec3f origin = offsetRayOrigin(hit.getIntersectionPoint(), geomNormal,
                                     reflectedDir, RAY_EPSILON);
      Ray reflectedRay(origin, reflectedDir);
      float newWeight = weight * reflectiveColor.Length();
      Hit reflectedHit(max_t, nullptr, Vec3f(0, 0, 0));
      float iorStack[MAX_IOR_DEPTH];
      for (int i = 0; i < iorDepth; i++)
        iorStack[i] = outsideIOR[i];
      Vec3f reflected = traceRayRecursive(reflectedRay, RAY_EPSILON, bounces + 1, newWeight, indexOfRefraction, reflectedHit, iorStack, iorDepth);
      RayTree::AddReflectedSegment(reflectedRay, 0.0f, reflectedHit.getT());
      color += componentMultiply(reflectiveColor, reflected);
    }

    Vec3f transparentColor = phong->getTransparentColor();
    if (hasPositive(transparentColor)) {
      float index_i, index_t;
      float nDotD = hit.getNormal().Dot3(ray.getDirection());
      float nextIOR;
      float iorStack[MAX_IOR_DEPTH];
      for (int i = 0; i < iorDepth; i++)
        iorStack[i] = outsideIOR[i];
      int nextDepth = iorDepth;

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
        Vec3f geomNormal = hit.getNormal();
        Vec3f origin = offsetRayOrigin(hit.getIntersectionPoint(), geomNormal,
                                       transmittedDir, RAY_EPSILON);
        Ray transmittedRay(origin, transmittedDir);
        float newWeight = weight * transparentColor.Length();
        Hit transmittedHit(max_t, nullptr, Vec3f(0, 0, 0));
        Vec3f transmitted = traceRayRecursive(transmittedRay, RAY_EPSILON, bounces + 1, newWeight, nextIOR, transmittedHit, iorStack, nextDepth);
        RayTree::AddTransmittedSegment(transmittedRay, 0.0f,
                                       transmittedHit.getT());
        color += componentMultiply(transparentColor, transmitted);
      }
    }
  }

  return color;
}
