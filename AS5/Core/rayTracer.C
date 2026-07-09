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
#include <math.h>
#include <assert.h>
#include <stdio.h>

const float RayTracer::RAY_EPSILON = 1e-4f;  // 射线偏移量
const int RayTracer::MAX_IOR_DEPTH = 16;  // 最大折射率深度
const float RayTracer::SHADOW_ATTENUATION_SCALE = 2.5f;  // 阴影衰减因子

// 构造光线追踪器
RayTracer::RayTracer(SceneParser *s, int max_bounces, float cutoff_weight,
                     bool shadows, bool shade_back, bool transparent_shadows,
                     int grid_nx, int grid_ny, int grid_nz, bool visualize_grid) {
  parser = s;  // 场景解析器
  maxBounces = max_bounces;
  cutoffWeight = cutoff_weight;
  castShadows = shadows;
  shadeBack = shade_back;
  transparentShadows = transparent_shadows;
  grid = NULL;
  visualizeGrid = visualize_grid;

  // 边界检查
  if (grid_nx <= 0 || grid_ny <= 0 || grid_nz <= 0)
    return;

  Group *sceneGroup = parser->getGroup();  // 场景组
  BoundingBox *sceneBounds = sceneGroup->getBoundingBox();  // 场景包围盒
  if (sceneBounds == NULL)
    return;

  // [DEBUG] 打印包围盒
  printf("=== Bounding Box Debug ===\n");
  printf("Scene bounding box: ");
  sceneBounds->Print();
  sceneGroup->debugPrintBoundingBox(0);

  grid = new Grid(sceneBounds, grid_nx, grid_ny, grid_nz);  // 创建体素网格
  sceneGroup->insertIntoGrid(grid, NULL);  // 将场景组插入体素网格

  // [DEBUG] 打印体素网格占用情况
  printf("=== Grid Rasterization Debug ===\n");
  grid->printOccupancy();
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

// 准备法线
Vec3f RayTracer::prepareNormal(const Ray &ray, const Hit &hit,
                               bool &backFacing) const {
  Vec3f normal = hit.getNormal();  // 法线
  backFacing = ray.getDirection().Dot3(normal) > 0.0f;
  if (shadeBack && backFacing)
    normal = normal * (-1.0f);  // 背面着色
  return normal;
}

// 阴影衰减
Vec3f RayTracer::getShadowAttenuation(const Vec3f &point,
                                      const Vec3f &geomNormal,
                                      const Vec3f &lightDir,
                                      float distanceToLight) const {
  Group *group = parser->getGroup(); // 场景组
  Vec3f origin = offsetRayOrigin(point, geomNormal, lightDir, RAY_EPSILON); // 起点用几何法线偏置
  Ray shadowRay(origin, lightDir); // 阴影射线

  const float maxDist = (distanceToLight < 1.0e20f)
      ? distanceToLight - RAY_EPSILON
      : 1.0e20f;

  // 透明阴影关闭则直接求交
  if (!transparentShadows) {
    float t;
    if (group->intersectShadow(shadowRay, RAY_EPSILON, maxDist, t, NULL)) {
      RayTree::AddShadowSegment(shadowRay, 0.0f, t);
      return Vec3f(0, 0, 0);
    }
    return Vec3f(1, 1, 1);
  }

  Vec3f attenuation(1, 1, 1); // 衰减
  float traveled = 0.0f; //  物体到光源的距离

  while (traveled < maxDist) {
    float t; // 阻挡物距离
    Material *blocker = NULL; // 阻挡物材质
    if (!group->intersectShadow(shadowRay, RAY_EPSILON, maxDist - traveled, t, &blocker))
      break; // 无阻挡物

    RayTree::AddShadowSegment(shadowRay, 0.0f, t); // 添加阴影段

    PhongMaterial *phong = (PhongMaterial *)blocker;
    if (!hasPositive(phong->getTransparentColor()))  // 透明色不为正向
      return Vec3f(0, 0, 0);

    // 进入透明体后再次求交得出射点 tExit
    Vec3f entry = shadowRay.pointAtParameter(t);
    Ray exitRay(entry + lightDir * RAY_EPSILON, lightDir);
    float tExit; // 出射点距离
    Material *exitMat = NULL; // 出射点材质
    if (!group->intersectShadow(exitRay, RAY_EPSILON, maxDist - traveled - t - RAY_EPSILON, tExit, &exitMat))
      return Vec3f(0, 0, 0); // 无出射点

    float thickness = tExit + RAY_EPSILON; // 厚度 = 内部光程
    attenuation = componentMultiply(
        attenuation, transmittanceThrough(phong->getTransparentColor(),
                                            thickness));
    if (isFullyBlocked(attenuation)) // 完全遮挡
      return Vec3f(0, 0, 0);

    traveled += t + thickness + RAY_EPSILON; // 更新 traveled
    origin = exitRay.pointAtParameter(tExit) + lightDir * RAY_EPSILON; // 更新起点
    shadowRay = Ray(origin, lightDir); // 更新阴影射线
  }

  return attenuation; // 返回衰减
}

// 局部着色
Vec3f RayTracer::computeLocalShading(const Ray &ray, const Hit &hit,
                                     const Vec3f &normal) const {
  Material *material = hit.getMaterial(); // 材质 
  Vec3f color = componentMultiply(parser->getAmbientLight(), material->getDiffuseColor()); // 环境光 + 漫反射光

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

    Hit shadedHit(hit.getT(), material, normal); // 反射/折射着色点
    color += material->Shade(ray, shadedHit, lightDir, lightColor); // 反射/折射着色
  }
  return color;
}

// 光线追踪
Vec3f RayTracer::traceRay(Ray &ray, float tmin, int bounces, float weight,
                          float indexOfRefraction, Hit &hit) const {
  float outsideIOR[MAX_IOR_DEPTH]; // 记录进入每一层透明体前的外围介质 IOR（嵌套折射）
  return traceRayRecursive(ray, tmin, bounces, weight, indexOfRefraction, hit,
                           outsideIOR, 0); // 递归
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

  // 如果可视化体素网格，则对体素网格做 DDA 求交，否则对场景几何做求交
  if (visualizeGrid && grid != NULL) {
    hitSomething = grid->intersect(ray, bestHit, tmin);
  } else {
    Group *group = parser->getGroup();
    hitSomething = group->intersect(ray, bestHit, tmin);
  }

  if (!hitSomething)
    return background;

  hit = bestHit; // 命中点
  RayTree::SetMainSegment(ray, 0.0f, hit.getT()); // 设置主段

  bool backFacing = false;
  Vec3f normal = prepareNormal(ray, hit, backFacing);
  if (!shadeBack && backFacing)
    return Vec3f(0, 0, 0);

  PhongMaterial *phong = (PhongMaterial *)hit.getMaterial();
  assert(phong != NULL);

  Vec3f color = computeLocalShading(ray, hit, normal); // 局部着色

  // 网格可视化做局部着色
  if (visualizeGrid && grid != NULL)
    return color;

  // I. 反射着色
  if (bounces < maxBounces && weight >= cutoffWeight) {
    Vec3f reflectiveColor = phong->getReflectiveColor(); // 反射色
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
      Vec3f reflected = traceRayRecursive(reflectedRay, RAY_EPSILON, bounces + 1, newWeight, indexOfRefraction, reflectedHit, iorStack, iorDepth); // 递归
      RayTree::AddReflectedSegment(reflectedRay, 0.0f, reflectedHit.getT()); // 添加反射段
      color += componentMultiply(reflectiveColor, reflected); // 反射色 + 反射命中
    }

    // II. 透明体着色
    Vec3f transparentColor = phong->getTransparentColor();  // 透明色
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
        index_t = phong->getIndexOfRefraction();
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
        Vec3f transmitted = traceRayRecursive(transmittedRay, RAY_EPSILON, bounces + 1, newWeight, nextIOR, transmittedHit, iorStack, nextDepth); // 递归
        RayTree::AddTransmittedSegment(transmittedRay, 0.0f,
                                       transmittedHit.getT()); // 添加折射段
        color += componentMultiply(transparentColor, transmitted); // 透明色 + 折射命中
      }
    }
  }

  return color; // 返回着色结果
}
