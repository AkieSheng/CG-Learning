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
#include "marchinginfo.h"
#include "object3dvector.h"
#include "raytracing_stats.h"
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
  intersectionMarkCounter = 0;


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
                                 Vec3f const&incoming)const -> Vec3f {
  float nDotI = normal.Dot3(incoming);
  return incoming - normal * (2.0f * nDotI);
}


auto RayTracer::transmittedDirection(Vec3f const&normal,
                                     Vec3f const&incoming,
                                     float index_i, float index_t,
                                     Vec3f &transmitted)const -> bool {
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

static auto cellExitT(MarchingInfo const&mi) -> float {
  return ::fminf(::fminf(mi.getTNextX(), mi.getTNextY()), mi.getTNextZ());
}



static const float CELL_T_EPSILON = 1e-3f;

static auto hitInCell(float t, float cellTMin, float cellTMax) -> bool {
  return t >= cellTMin - CELL_T_EPSILON && t <= cellTMax + CELL_T_EPSILON;
}


auto RayTracer::prepareNormal(Ray const&ray, Hit const&hit,
                               bool &backFacing)const -> Vec3f {
  Vec3f normal = hit.getNormal();
  backFacing = ray.getDirection().Dot3(normal) > 0.0f;
  if (shadeBack && backFacing)
    normal = normal * (-1.0f);
  return normal;
}


auto RayTracer::rayCast(Ray const&ray, Hit &hit, float tmin)const -> bool {
  return parser->getGroup()->intersect(ray, hit, tmin);
}

auto RayTracer::beginIntersectionMarking()const -> void {
  intersectionMarkCounter++;
}

auto RayTracer::isMarked(Object3D const*obj)const -> bool {
  return obj->getIntersectionMark() == intersectionMarkCounter;
}

auto RayTracer::markObject(Object3D *obj)const -> void {
  obj->setIntersectionMark(intersectionMarkCounter);
}


auto RayTracer::castSceneIntersect(Ray const&ray, Hit &hit, float tmin)const -> bool {
  if (visualizeGrid && grid != nullptr)
    return grid->intersect(ray, hit, tmin);
  if (grid != nullptr)
    return rayCastFast(ray, hit, tmin);
  return rayCast(ray, hit, tmin);
}


auto RayTracer::rayCastFast(Ray const&ray, Hit &hit, float tmin)const -> bool {
  const float max_t = 1.0e30f;
  Hit bestHit(max_t, nullptr, Vec3f(0, 0, 0));
  bool found = false;

  beginIntersectionMarking();


  Object3DVector const&infinite = grid->getInfiniteObjects();
  for (int i = 0; i < infinite.getNumObjects(); i++) {
    if (infinite.getObject(i)->intersect(ray, bestHit, tmin))
      found = true;
  }


  MarchingInfo mi;
  grid->initializeRayMarch(mi, ray, tmin);
  if (!mi.isValid()) {
    if (found)
      hit = bestHit;
    return found;
  }


  while (mi.getTMin() <= mi.getTExit()) {

    int ci = mi.getI();
    int cj = mi.getJ();
    int ck = mi.getK();
    if (!grid->inBounds(ci, cj, ck))
      break;

    float cellTMin = mi.getTMin();
    float cellTMax = cellExitT(mi);

    Object3DVector *cell = grid->getCell(ci, cj, ck);

    for (int o = 0; o < cell->getNumObjects(); o++) {
      Object3D *obj = cell->getObject(o);
      if (!isMarked(obj)) {
        markObject(obj);

        Hit candidate(max_t, nullptr, Vec3f(0, 0, 0));
        if (obj->intersect(ray, candidate, tmin))
          obj->setMarkedIntersection(candidate);
        else
          obj->clearMarkedIntersection();
      }

      if (obj->getHasMarkedIntersection()) {
        Hit const&candidate = obj->getMarkedHit();
        float t = candidate.getT();
        if (hitInCell(t, cellTMin, cellTMax) && t < bestHit.getT()) {
          bestHit = candidate;
          found = true;
        }
      }
    }
    if (found && bestHit.getT() <= cellTMax + CELL_T_EPSILON) {
      hit = bestHit;
      return true;
    }

    mi.nextCell();
  }

  if (found)
    hit = bestHit;
  return found;
}


auto RayTracer::castSceneShadow(Ray const&ray, float tmin, float tmax, float &t,
                                Material **outMaterial)const -> bool {
  if (grid == nullptr || visualizeGrid)
    return parser->getGroup()->intersectShadow(ray, tmin, tmax, t, outMaterial);
  return rayCastShadow(ray, tmin, tmax, t, outMaterial);
}


auto RayTracer::rayCastShadow(Ray const&ray, float tmin, float tmax, float &t,
                              Material **outMaterial)const -> bool {
  Object3DVector const&infinite = grid->getInfiniteObjects();

  beginIntersectionMarking();


  if (outMaterial == nullptr) {
    for (int i = 0; i < infinite.getNumObjects(); i++) {
      float hitT;
      if (infinite.getObject(i)->intersectShadow(ray, tmin, tmax, hitT, nullptr))
        return true;
    }

    MarchingInfo mi;
    grid->initializeRayMarch(mi, ray, tmin);
    while (mi.isValid() && mi.getTMin() <= mi.getTExit() && mi.getTMin() <= tmax) {
      int ci = mi.getI();
      int cj = mi.getJ();
      int ck = mi.getK();
      if (!grid->inBounds(ci, cj, ck))
        break;

      float cellTMin = mi.getTMin();
      float cellTMax = ::fminf(cellExitT(mi), tmax);
      Object3DVector *cell = grid->getCell(ci, cj, ck);

      for (int o = 0; o < cell->getNumObjects(); o++) {
        Object3D *obj = cell->getObject(o);
        if (!isMarked(obj)) {
          markObject(obj);
          float hitT;
          if (obj->intersectShadow(ray, tmin, tmax, hitT, nullptr))
            obj->setMarkedIntersection(Hit(hitT, nullptr, Vec3f(0, 0, 0)));
          else
            obj->clearMarkedIntersection();
        }

        if (obj->getHasMarkedIntersection()) {
          float hitT = obj->getMarkedHit().getT();
          if (hitInCell(hitT, cellTMin, cellTMax)) {
            t = hitT;
            return true;
          }
        }
      }
      mi.nextCell();
    }
    return false;
  }

  bool found = false;
  float bestT = tmax;
  Material *bestMat = nullptr;


  for (int i = 0; i < infinite.getNumObjects(); i++) {
    float hitT;
    Material *hitMat = nullptr;
    if (infinite.getObject(i)->intersectShadow(ray, tmin, bestT, hitT, &hitMat)) {
      bestT = hitT;
      bestMat = hitMat;
      found = true;
    }
  }

  MarchingInfo mi;
  grid->initializeRayMarch(mi, ray, tmin);

  while (mi.isValid() && mi.getTMin() <= mi.getTExit() && mi.getTMin() <= tmax) {
    int ci = mi.getI();
    int cj = mi.getJ();
    int ck = mi.getK();
    if (!grid->inBounds(ci, cj, ck))
      break;

    float cellTMin = mi.getTMin();
    float cellTMax = ::fminf(cellExitT(mi), tmax);
    Object3DVector *cell = grid->getCell(ci, cj, ck);
    for (int o = 0; o < cell->getNumObjects(); o++) {
      Object3D *obj = cell->getObject(o);
      if (!isMarked(obj)) {
        markObject(obj);
        float hitT;
        Material *hitMat = nullptr;

        if (obj->intersectShadow(ray, tmin, tmax, hitT, &hitMat))
          obj->setMarkedIntersection(Hit(hitT, hitMat, Vec3f(0, 0, 0)));
        else
          obj->clearMarkedIntersection();
      }

      if (obj->getHasMarkedIntersection()) {
        Hit const&marked = obj->getMarkedHit();
        float hitT = marked.getT();
        if (hitInCell(hitT, cellTMin, cellTMax) && hitT < bestT) {
          bestT = hitT;
          bestMat = marked.getMaterial();
          found = true;
        }
      }
    }
    mi.nextCell();
  }

  if (found) {
    t = bestT;
    *outMaterial = bestMat;
  }
  return found;
}


auto RayTracer::getShadowAttenuation(Vec3f const&point,
                                      Vec3f const&geomNormal,
                                      Vec3f const&lightDir,
                                      float distanceToLight)const -> Vec3f {
  Vec3f origin = offsetRayOrigin(point, geomNormal, lightDir, RAY_EPSILON);
  Ray shadowRay(origin, lightDir);
  RayTracingStats::IncrementNumShadowRays();

  const float maxDist = (distanceToLight < 1.0e20f)
      ? distanceToLight - RAY_EPSILON
      : 1.0e20f;


  if (!transparentShadows) {
    float t;
    if (castSceneShadow(shadowRay, RAY_EPSILON, maxDist, t, nullptr)) {
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
    if (!castSceneShadow(shadowRay, RAY_EPSILON, maxDist - traveled, t, &blocker))
      break;

    RayTree::AddShadowSegment(shadowRay, 0.0f, t);

    Material *blockerMat = blocker;
    Vec3f blockerPoint = shadowRay.pointAtParameter(t);
    if (!hasPositive(blockerMat->getTransparentColor(blockerPoint)))
      return Vec3f(0, 0, 0);


    Vec3f entry = blockerPoint;
    Ray exitRay(entry + lightDir * RAY_EPSILON, lightDir);
    float tExit;
    Material *exitMat = nullptr;
    if (!castSceneShadow(exitRay, RAY_EPSILON, maxDist - traveled - t - RAY_EPSILON, tExit, &exitMat))
      return Vec3f(0, 0, 0);

    float thickness = tExit + RAY_EPSILON;
    attenuation = componentMultiply(
        attenuation, transmittanceThrough(blockerMat->getTransparentColor(entry),
                                            thickness));
    if (isFullyBlocked(attenuation))
      return Vec3f(0, 0, 0);

    traveled += t + thickness + RAY_EPSILON;
    origin = exitRay.pointAtParameter(tExit) + lightDir * RAY_EPSILON;
    shadowRay = Ray(origin, lightDir);
    RayTracingStats::IncrementNumShadowRays();
  }

  return attenuation;
}


auto RayTracer::computeLocalShading(Ray const&ray, Hit const&hit,
                                     Vec3f const&normal)const -> Vec3f {
  Material *material = hit.getMaterial();
  Vec3f worldPoint = hit.getIntersectionPoint();
  Vec3f color = componentMultiply(parser->getAmbientLight(),
                                material->getDiffuseColor(worldPoint));


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

    Hit shadedHit(hit);
    shadedHit.set(hit.getT(), material, normal, ray);
    color += material->Shade(ray, shadedHit, lightDir, lightColor);
  }
  return color;
}


auto RayTracer::traceRay(Ray &ray, float tmin, int bounces, float weight,
                          float indexOfRefraction, Hit &hit)const -> Vec3f {
  RayTracingStats::IncrementNumNonShadowRays();
  float outsideIOR[MAX_IOR_DEPTH];
  return traceRayRecursive(ray, tmin, bounces, weight, indexOfRefraction, hit,
                           outsideIOR, 0);
}


auto RayTracer::traceRayRecursive(Ray &ray, float tmin, int bounces,
                                   float weight, float indexOfRefraction,
                                   Hit &hit, float const*outsideIOR,
                                   int iorDepth)const -> Vec3f {
  Vec3f background = parser->getBackgroundColor();

  if (bounces > maxBounces || weight < cutoffWeight)
    return background;

  const float max_t = 1.0e30f;
  Hit bestHit(max_t, nullptr, Vec3f(0, 0, 0));
  bool hitSomething = false;


  hitSomething = castSceneIntersect(ray, bestHit, tmin);

  if (!hitSomething)
    return background;

  hit = bestHit;
  RayTree::SetMainSegment(ray, 0.0f, hit.getT());

  bool backFacing = false;
  Vec3f normal = prepareNormal(ray, hit, backFacing);
  if (!shadeBack && backFacing)
    return Vec3f(0, 0, 0);

  Material *material = hit.getMaterial();
  assert(material != nullptr);
  Vec3f worldPoint = hit.getIntersectionPoint();

  Vec3f color = computeLocalShading(ray, hit, normal);


  if (visualizeGrid && grid != nullptr)
    return color;


  if (bounces < maxBounces && weight >= cutoffWeight) {
    Vec3f reflectiveColor = material->getReflectiveColor(worldPoint);
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
      RayTracingStats::IncrementNumNonShadowRays();
      Vec3f reflected = traceRayRecursive(reflectedRay, RAY_EPSILON, bounces + 1, newWeight, indexOfRefraction, reflectedHit, iorStack, iorDepth);
      RayTree::AddReflectedSegment(reflectedRay, 0.0f, reflectedHit.getT());
      color += componentMultiply(reflectiveColor, reflected);
    }


    Vec3f transparentColor = material->getTransparentColor(worldPoint);
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
        index_t = material->getIndexOfRefraction(worldPoint);
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
        RayTracingStats::IncrementNumNonShadowRays();
        Vec3f transmitted = traceRayRecursive(transmittedRay, RAY_EPSILON, bounces + 1, newWeight, nextIOR, transmittedHit, iorStack, nextDepth);
        RayTree::AddTransmittedSegment(transmittedRay, 0.0f,
                                       transmittedHit.getT());
        color += componentMultiply(transparentColor, transmitted);
      }
    }
  }

  return color;
}
