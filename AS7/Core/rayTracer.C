#include "rayTracer.h"
#include "scene_parser.h"
#include "group.h"
#include "material.h"
#include "light.h"
#include "grid.h"
#include "boundingbox.h"
#include "marchinginfo.h"
#include "object3dvector.h"
#include "raytracing_stats.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>

const float RayTracer::RAY_EPSILON = 1e-4f;  // 射线偏移量
const int RayTracer::MAX_IOR_DEPTH = 16;  // 最大折射率深度
const float RayTracer::SHADOW_ATTENUATION_SCALE = 2.5f;  // 阴影衰减因子

// 构造光线追踪器
RayTracer::RayTracer(SceneParser *s, int max_bounces, float cutoff_weight,
                     bool shadows, bool shade_back, bool transparent_shadows,
                     int grid_nx, int grid_ny, int grid_nz) {
  parser = s;  // 场景解析器
  maxBounces = max_bounces;
  cutoffWeight = cutoff_weight;
  castShadows = shadows;
  shadeBack = shade_back;
  transparentShadows = transparent_shadows;
  grid = NULL;
  intersectionMarkCounter = 0;

  // 边界检查
  if (grid_nx <= 0 || grid_ny <= 0 || grid_nz <= 0)
    return;

  Group *sceneGroup = parser->getGroup();  // 场景组
  BoundingBox *sceneBounds = sceneGroup->getBoundingBox();  // 场景包围盒
  if (sceneBounds == NULL)
    return;

  grid = new Grid(sceneBounds, grid_nx, grid_ny, grid_nz);  // 创建体素网格
  sceneGroup->insertIntoGrid(grid, NULL);  // 将场景组插入体素网格
}

RayTracer::~RayTracer() {
  delete grid;
}

// 颜色向量逐分量相乘
Vec3f RayTracer::componentMultiply(const Vec3f &a, const Vec3f &b) {
  return Vec3f(a.x() * b.x(), a.y() * b.y(), a.z() * b.z());
}

// 是否为正向
bool RayTracer::hasPositive(const Vec3f &c) {
  return c.x() > 0.0f || c.y() > 0.0f || c.z() > 0.0f;
}

// 阴影遮挡判定
bool RayTracer::isFullyBlocked(const Vec3f &attenuation) {
  return attenuation.x() <= 0.0f && attenuation.y() <= 0.0f &&
         attenuation.z() <= 0.0f;
}

// Beer-Lambert：transparentColor 为透射色，吸收系数 κ = 1 - transparentColor
Vec3f RayTracer::transmittanceThrough(const Vec3f &transparentColor,
                                      float distance) {
  float d = distance * SHADOW_ATTENUATION_SCALE;
  return Vec3f(
      expf(-(1.0f - transparentColor.x()) * d),
      expf(-(1.0f - transparentColor.y()) * d),
      expf(-(1.0f - transparentColor.z()) * d));
}

// 镜像方向：R = I - 2(N·I)N
Vec3f RayTracer::mirrorDirection(const Vec3f &normal,
                                 const Vec3f &incoming) const {
  float nDotI = normal.Dot3(incoming);
  return incoming - normal * (2.0f * nDotI);
}

// Snell 折射方向；eta = index_i / index_t
bool RayTracer::transmittedDirection(const Vec3f &normal,
                                     const Vec3f &incoming,
                                     float index_i, float index_t,
                                     Vec3f &transmitted) const {
  Vec3f n = normal;  // 法线
  float cosi = n.Dot3(incoming);  // 入射角
  if (cosi > 0.0f) {  // 入射角为负，法线取反
    n = n * (-1.0f);
    cosi = -cosi;
  }
  float eta = index_i / index_t;
  float k = 1.0f - eta * eta * (1.0f - cosi * cosi);
  if (k < 0.0f)  // 全反射
    return false;
  transmitted = incoming * eta - n * (eta * cosi + sqrtf(k));  // T = ηI - (η(I·N)+√k)N
  transmitted.Normalize();
  return true;  // 折射存在
}

// 沿 dir 方向将起点偏置到几何表面外侧，避免自相交
static Vec3f offsetRayOrigin(const Vec3f &point, const Vec3f &geomNormal,
                             const Vec3f &dir, float eps) {
  Vec3f nudge = (dir.Dot3(geomNormal) > 0.0f) ? geomNormal * eps
                                              : geomNormal * (-eps);
  return point + nudge;
}

static float cellExitT(const MarchingInfo &mi) {
  return fminf(fminf(mi.getTNextX(), mi.getTNextY()), mi.getTNextZ());
}

// 地板等贴在 cell 出口面的图元：几何 t 与 DDA 的 t_next 有浮点偏差
// （发现半开/严格闭区间都会漏交，下一格又可能已出界导致了点状缺失）。
static const float CELL_T_EPSILON = 1e-3f;

static bool hitInCell(float t, float cellTMin, float cellTMax) {
  return t >= cellTMin - CELL_T_EPSILON && t <= cellTMax + CELL_T_EPSILON;
}

// 准备法线
Vec3f RayTracer::prepareNormal(const Ray &ray, const Hit &hit,
                               bool &backFacing) const {
  Vec3f normal = hit.getNormal();  // 法线
  backFacing = ray.getDirection().Dot3(normal) > 0.0f;
  if (shadeBack && backFacing)
    normal = normal * (-1.0f);  // 背面着色
  return normal;
}

// 无加速求交（遍历所有物体）
bool RayTracer::rayCast(const Ray &ray, Hit &hit, float tmin) const {
  return parser->getGroup()->intersect(ray, hit, tmin);
}

void RayTracer::beginIntersectionMarking() const {
  intersectionMarkCounter++;
}

bool RayTracer::isMarked(const Object3D *obj) const {
  return obj->getIntersectionMark() == intersectionMarkCounter;
}

void RayTracer::markObject(Object3D *obj) const {
  obj->setIntersectionMark(intersectionMarkCounter);
}

// 场景求交
bool RayTracer::castSceneIntersect(const Ray &ray, Hit &hit, float tmin) const {
  if (grid != NULL)
    return rayCastFast(ray, hit, tmin);
  return rayCast(ray, hit, tmin);
}

// 网格加速求交（遍历图元）
bool RayTracer::rayCastFast(const Ray &ray, Hit &hit, float tmin) const {
  const float max_t = 1.0e30f;
  Hit bestHit(max_t, NULL, Vec3f(0, 0, 0));
  bool found = false;

  beginIntersectionMarking();

  // 遍历无限图元
  const Object3DVector &infinite = grid->getInfiniteObjects();
  for (int i = 0; i < infinite.getNumObjects(); i++) {
    if (infinite.getObject(i)->intersect(ray, bestHit, tmin))
      found = true;
  }

  // 初始化射线步进
  MarchingInfo mi;
  grid->initializeRayMarch(mi, ray, tmin);
  if (!mi.isValid()) {
    if (found)
      hit = bestHit;
    return found;
  }

  // 射线步进
  while (mi.getTMin() <= mi.getTExit()) {
    // 获取体素索引
    int ci = mi.getI();
    int cj = mi.getJ();
    int ck = mi.getK();
    if (!grid->inBounds(ci, cj, ck))
      break;

    float cellTMin = mi.getTMin();  // 体素进入时间
    float cellTMax = cellExitT(mi);  // 体素离开时间

    Object3DVector *cell = grid->getCell(ci, cj, ck);
    // 遍历图元找到交点
    for (int o = 0; o < cell->getNumObjects(); o++) {
      Object3D *obj = cell->getObject(o);
      if (!isMarked(obj)) {
        markObject(obj);
        // 用 Hit 缓存交点信息
        Hit candidate(max_t, NULL, Vec3f(0, 0, 0));
        if (obj->intersect(ray, candidate, tmin))
          obj->setMarkedIntersection(candidate);
        else
          obj->clearMarkedIntersection();
      }

      if (obj->getHasMarkedIntersection()) {
        const Hit &candidate = obj->getMarkedHit();
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

// 场景阴影求交
bool RayTracer::castSceneShadow(const Ray &ray, float tmin, float tmax, float &t,
                                Material **outMaterial) const {
  if (grid == NULL)
    return parser->getGroup()->intersectShadow(ray, tmin, tmax, t, outMaterial);  // 遍历所有物体，返回最近交点
  return rayCastShadow(ray, tmin, tmax, t, outMaterial);
}

// 阴影求交（遍历图元）
bool RayTracer::rayCastShadow(const Ray &ray, float tmin, float tmax, float &t,
                              Material **outMaterial) const {
  const Object3DVector &infinite = grid->getInfiniteObjects();

  beginIntersectionMarking();

  // 遍历无限图元
  if (outMaterial == NULL) {
    for (int i = 0; i < infinite.getNumObjects(); i++) {
      float hitT;
      if (infinite.getObject(i)->intersectShadow(ray, tmin, tmax, hitT, NULL))  // 射线-物体求交阴影
        return true;
    }

    MarchingInfo mi;  // 射线步进
    grid->initializeRayMarch(mi, ray, tmin);
    while (mi.isValid() && mi.getTMin() <= mi.getTExit() && mi.getTMin() <= tmax) {
      int ci = mi.getI();  // 体素索引
      int cj = mi.getJ();
      int ck = mi.getK();
      if (!grid->inBounds(ci, cj, ck))
        break;

      float cellTMin = mi.getTMin();
      float cellTMax = fminf(cellExitT(mi), tmax);
      Object3DVector *cell = grid->getCell(ci, cj, ck);
      // 遍历图元找到交点
      for (int o = 0; o < cell->getNumObjects(); o++) {
        Object3D *obj = cell->getObject(o);
        if (!isMarked(obj)) {
          markObject(obj);
          float hitT;
          if (obj->intersectShadow(ray, tmin, tmax, hitT, NULL))
            obj->setMarkedIntersection(Hit(hitT, NULL, Vec3f(0, 0, 0)));
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
    return false;  // 无交点
  }

  bool found = false;
  float bestT = tmax;  // 最小交点时间
  Material *bestMat = NULL;

  // 遍历无限图元
  for (int i = 0; i < infinite.getNumObjects(); i++) {
    float hitT;
    Material *hitMat = NULL;
    if (infinite.getObject(i)->intersectShadow(ray, tmin, bestT, hitT, &hitMat)) {
      bestT = hitT;
      bestMat = hitMat;
      found = true;
    }
  }

  MarchingInfo mi;
  grid->initializeRayMarch(mi, ray, tmin);
  // 循环进行射线步进
  while (mi.isValid() && mi.getTMin() <= mi.getTExit() && mi.getTMin() <= tmax) {
    int ci = mi.getI();
    int cj = mi.getJ();
    int ck = mi.getK();
    if (!grid->inBounds(ci, cj, ck))
      break;

    float cellTMin = mi.getTMin();
    float cellTMax = fminf(cellExitT(mi), tmax);
    Object3DVector *cell = grid->getCell(ci, cj, ck);
    for (int o = 0; o < cell->getNumObjects(); o++) {
      Object3D *obj = cell->getObject(o);
      if (!isMarked(obj)) {
        markObject(obj);
        float hitT;
        Material *hitMat = NULL;
        // 用 tmax 缓存真实交点
        if (obj->intersectShadow(ray, tmin, tmax, hitT, &hitMat))
          obj->setMarkedIntersection(Hit(hitT, hitMat, Vec3f(0, 0, 0)));
        else
          obj->clearMarkedIntersection();
      }

      if (obj->getHasMarkedIntersection()) {
        const Hit &marked = obj->getMarkedHit();
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

  if (found) {  // 有交点
    t = bestT;
    *outMaterial = bestMat;
  }
  return found;
}

// 阴影衰减
Vec3f RayTracer::getShadowAttenuation(const Vec3f &point,
                                      const Vec3f &geomNormal,
                                      const Vec3f &lightDir,
                                      float distanceToLight) const {
  Vec3f origin = offsetRayOrigin(point, geomNormal, lightDir, RAY_EPSILON); // 起点用几何法线偏置
  Ray shadowRay(origin, lightDir); // 阴影射线
  RayTracingStats::IncrementNumShadowRays();

  const float maxDist = (distanceToLight < 1.0e20f)
      ? distanceToLight - RAY_EPSILON
      : 1.0e20f;

  // 透明阴影关闭则直接求交
  if (!transparentShadows) {
    float t;
    if (castSceneShadow(shadowRay, RAY_EPSILON, maxDist, t, NULL))
      return Vec3f(0, 0, 0);
    return Vec3f(1, 1, 1);
  }

  Vec3f attenuation(1, 1, 1); // 衰减
  float traveled = 0.0f; //  物体到光源的距离

  while (traveled < maxDist) {
    float t; // 阻挡物距离
    Material *blocker = NULL; // 阻挡物材质
    if (!castSceneShadow(shadowRay, RAY_EPSILON, maxDist - traveled, t, &blocker))
      break; // 无阻挡物

    Material *blockerMat = blocker;
    Vec3f blockerPoint = shadowRay.pointAtParameter(t); // 阻挡物点
    if (!hasPositive(blockerMat->getTransparentColor(blockerPoint)))  // 透明色不为正向
      return Vec3f(0, 0, 0);

    // 进入透明体后再次求交得出射点 tExit
    Vec3f entry = blockerPoint;
    Ray exitRay(entry + lightDir * RAY_EPSILON, lightDir);
    float tExit; // 出射点距离
    Material *exitMat = NULL; // 出射点材质
    if (!castSceneShadow(exitRay, RAY_EPSILON, maxDist - traveled - t - RAY_EPSILON, tExit, &exitMat))
      return Vec3f(0, 0, 0); // 无出射点

    float thickness = tExit + RAY_EPSILON; // 厚度 = 内部光程
    attenuation = componentMultiply(
        attenuation, transmittanceThrough(blockerMat->getTransparentColor(entry),
                                            thickness));
    if (isFullyBlocked(attenuation)) // 完全遮挡
      return Vec3f(0, 0, 0);

    traveled += t + thickness + RAY_EPSILON; // 更新 traveled
    origin = exitRay.pointAtParameter(tExit) + lightDir * RAY_EPSILON; // 更新起点
    shadowRay = Ray(origin, lightDir); // 更新阴影射线
    RayTracingStats::IncrementNumShadowRays();
  }

  return attenuation; // 返回衰减
}

// 局部着色
Vec3f RayTracer::computeLocalShading(const Ray &ray, const Hit &hit,
                                     const Vec3f &normal) const {
  Material *material = hit.getMaterial();  // 材质
  Vec3f worldPoint = hit.getIntersectionPoint();  // 世界点
  Vec3f color = componentMultiply(parser->getAmbientLight(),
                                material->getDiffuseColor(worldPoint));  // 环境光 + 漫反射光

  // 逐光源着色
  for (int i = 0; i < parser->getNumLights(); i++) {
    Vec3f lightDir, lightColor;
    float distanceToLight;
    parser->getLight(i)->getIllumination(hit.getIntersectionPoint(), lightDir, lightColor, distanceToLight);

    if (castShadows) {
      Vec3f shadowAtten = getShadowAttenuation(hit.getIntersectionPoint(),
                                               hit.getNormal(), lightDir,
                                               distanceToLight);  // 阴影衰减
      if (isFullyBlocked(shadowAtten)) // 完全遮挡
        continue;
      lightColor = componentMultiply(lightColor, shadowAtten);
    }

    Hit shadedHit(hit);
    shadedHit.set(hit.getT(), material, normal, ray);
    color += material->Shade(ray, shadedHit, lightDir, lightColor);
  }
  return color;
}

// 光线追踪
Vec3f RayTracer::traceRay(Ray &ray, float tmin, int bounces, float weight,
                          float indexOfRefraction, Hit &hit) const {
  RayTracingStats::IncrementNumNonShadowRays();
  float outsideIOR[MAX_IOR_DEPTH]; // 记录进入每一层透明体前的外围介质 IOR（嵌套折射）
  return traceRayRecursive(ray, tmin, bounces, weight, indexOfRefraction, hit,
                           outsideIOR, 0); // 递归光线追踪
}

// 递归光线追踪
Vec3f RayTracer::traceRayRecursive(Ray &ray, float tmin, int bounces,
                                   float weight, float indexOfRefraction,
                                   Hit &hit, const float *outsideIOR,
                                   int iorDepth) const {
  Vec3f background = parser->getBackgroundColor(); // 背景色

  if (bounces > maxBounces || weight < cutoffWeight)  // 超出最大反弹次数或权重小于截断权重
    return background;

  const float max_t = 1.0e30f;
  Hit bestHit(max_t, NULL, Vec3f(0, 0, 0)); // 命中点
  bool hitSomething = false;

  // 对场景几何做求交
  hitSomething = castSceneIntersect(ray, bestHit, tmin);

  if (!hitSomething)
    return background;

  hit = bestHit; // 命中点

  bool backFacing = false;
  Vec3f normal = prepareNormal(ray, hit, backFacing);
  if (!shadeBack && backFacing)
    return Vec3f(0, 0, 0);

  Material *material = hit.getMaterial();
  assert(material != NULL);
  Vec3f worldPoint = hit.getIntersectionPoint();

  Vec3f color = computeLocalShading(ray, hit, normal); // 局部着色

  // I. 反射着色
  if (bounces < maxBounces && weight >= cutoffWeight) {
    Vec3f reflectiveColor = material->getReflectiveColor(worldPoint); // 反射色
    if (hasPositive(reflectiveColor)) {
      Vec3f geomNormal = hit.getNormal(); // 几何法线
      Vec3f reflectedDir = mirrorDirection(normal, ray.getDirection()); // 反射方向
      reflectedDir.Normalize();
      Vec3f origin = offsetRayOrigin(hit.getIntersectionPoint(), geomNormal,
                                     reflectedDir, RAY_EPSILON); // 起点
      Ray reflectedRay(origin, reflectedDir); // 反射射线
      float newWeight = weight * reflectiveColor.Length(); // 新权重
      Hit reflectedHit(max_t, NULL, Vec3f(0, 0, 0)); // 反射命中
      // 记录进入每一层透明体前的外围介质 IOR（嵌套折射）
      float iorStack[MAX_IOR_DEPTH];
      for (int i = 0; i < iorDepth; i++)
        iorStack[i] = outsideIOR[i];
      RayTracingStats::IncrementNumNonShadowRays(); // 递归次数 + 1
      Vec3f reflected = traceRayRecursive(reflectedRay, RAY_EPSILON, bounces + 1, newWeight, indexOfRefraction, reflectedHit, iorStack, iorDepth); // 递归
      color += componentMultiply(reflectiveColor, reflected); // 反射色 + 反射命中
    }

    // II. 透明体着色
    Vec3f transparentColor = material->getTransparentColor(worldPoint);  // 透明色
    if (hasPositive(transparentColor)) {
      float index_i, index_t; // 入射 IOR 和出射 IOR
      float nDotD = hit.getNormal().Dot3(ray.getDirection()); // 法线与方向点积
      float nextIOR; // 下一层 IOR
      // 记录进入每一层透明体前的外围介质 IOR（嵌套折射）
      float iorStack[MAX_IOR_DEPTH];
      for (int i = 0; i < iorDepth; i++)
        iorStack[i] = outsideIOR[i];
      int nextDepth = iorDepth;  // 下一层深度
      
      if (nDotD < 0.0f) {
        // 由外入内：index_i 为当前介质，index_t 为物体 IOR
        index_i = indexOfRefraction;
        index_t = material->getIndexOfRefraction(worldPoint);
        assert(iorDepth < MAX_IOR_DEPTH);
        iorStack[iorDepth] = indexOfRefraction;
        nextIOR = index_t;
        nextDepth = iorDepth + 1;
      } else {
        // 由内出外：index_i 为当前介质，index_t 为上一层介质 IOR 或真空 1.0
        index_i = indexOfRefraction;
        index_t = (iorDepth > 0) ? outsideIOR[iorDepth - 1] : 1.0f;
        nextIOR = index_t;
        nextDepth = iorDepth - 1;
      }

      // III. 折射着色
      Vec3f transmittedDir; // 折射方向
      if (transmittedDirection(normal, ray.getDirection(), index_i, index_t,
                               transmittedDir)) {
        Vec3f geomNormal = hit.getNormal(); // 几何法线
        Vec3f origin = offsetRayOrigin(hit.getIntersectionPoint(), geomNormal,
                                       transmittedDir, RAY_EPSILON); // 起点
        Ray transmittedRay(origin, transmittedDir); // 折射射线
        float newWeight = weight * transparentColor.Length(); // 新权重
        Hit transmittedHit(max_t, NULL, Vec3f(0, 0, 0)); // 折射命中
        RayTracingStats::IncrementNumNonShadowRays(); // 递归次数 + 1
        Vec3f transmitted = traceRayRecursive(transmittedRay, RAY_EPSILON, bounces + 1, newWeight, nextIOR, transmittedHit, iorStack, nextDepth); // 递归
        color += componentMultiply(transparentColor, transmitted); // 透明色 + 折射命中
      }
    }
  }

  return color; // 返回着色结果
}
