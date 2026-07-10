#ifndef _RAY_TRACER_H_
#define _RAY_TRACER_H_

#include "vectors.h"
#include "ray.h"
#include "hit.h"

class SceneParser;
class Material;
class Grid;
class Object3D;

// 递归光线追踪器
class RayTracer {

public:
  RayTracer(SceneParser *s, int max_bounces, float cutoff_weight,
            bool shadows, bool shade_back, bool transparent_shadows,
            int grid_nx, int grid_ny, int grid_nz);
  ~RayTracer();

  // 获取体素网格
  Grid *getGrid() const { return grid; }

  // 沿射线计算 radiance
  Vec3f traceRay(Ray &ray, float tmin, int bounces, float weight,
                 float indexOfRefraction, Hit &hit) const;

private:
  SceneParser *parser;  // 场景解析器
  Grid *grid;  // 均匀体素网格
  int maxBounces;  // 最大反弹次数
  float cutoffWeight;  // 截断权重
  bool castShadows;  // 是否投射阴影
  bool shadeBack;  // 是否背面着色
  bool transparentShadows;  // 是否透明阴影
  mutable int intersectionMarkCounter;  // grid marking 计数器

  static const float RAY_EPSILON;  // 射线偏移量
  static const int MAX_IOR_DEPTH;  // 最大折射率深度
  static const float SHADOW_ATTENUATION_SCALE;  // 阴影衰减因子

  Vec3f mirrorDirection(const Vec3f &normal, const Vec3f &incoming) const;  // 镜像方向
  bool transmittedDirection(const Vec3f &normal, const Vec3f &incoming,
                            float index_i, float index_t,
                            Vec3f &transmitted) const;  // 折射方向

  Vec3f prepareNormal(const Ray &ray, const Hit &hit, bool &backFacing) const;  // 准备法线
  Vec3f getShadowAttenuation(const Vec3f &point, const Vec3f &geomNormal,
                             const Vec3f &lightDir,
                             float distanceToLight) const;  // 阴影衰减
  Vec3f computeLocalShading(const Ray &ray, const Hit &hit,
                            const Vec3f &normal) const;  // 局部着色

  bool rayCast(const Ray &ray, Hit &hit, float tmin) const;  // 无加速求交
  bool rayCastFast(const Ray &ray, Hit &hit, float tmin) const;  // 网格加速求交
  bool rayCastShadow(const Ray &ray, float tmin, float tmax, float &t,
                     Material **outMaterial) const;  // 阴影求交

  bool castSceneIntersect(const Ray &ray, Hit &hit, float tmin) const;  // 场景求交
  bool castSceneShadow(const Ray &ray, float tmin, float tmax, float &t,
                       Material **outMaterial) const;  // 场景阴影求交

  void beginIntersectionMarking() const;  // 射线求交标记
  bool isMarked(const Object3D *obj) const;
  void markObject(Object3D *obj) const; // 标记物体

  Vec3f traceRayRecursive(Ray &ray, float tmin, int bounces, float weight,
                          float indexOfRefraction, Hit &hit,
                          const float *outsideIOR, int iorDepth) const;  // 递归光线追踪

  static Vec3f componentMultiply(const Vec3f &a, const Vec3f &b);  // 颜色向量逐分量相乘
  static bool hasPositive(const Vec3f &c);  // 是否为正向
  static Vec3f transmittanceThrough(const Vec3f &transparentColor,
                                    float distance);  // 半透明物体内的 Beer-Lambert 衰减
  static bool isFullyBlocked(const Vec3f &attenuation);  // 阴影遮挡判定
};

#endif
