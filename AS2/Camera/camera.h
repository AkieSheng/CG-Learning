#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "ray.h"
#include "vectors.h"

// 相机抽象基类
class Camera {

public:
  virtual ~Camera() {}
  virtual Ray generateRay(Vec2f point) = 0;  // 由屏幕坐标 [0,1]^2 生成射线
  virtual float getTMin() const = 0;         // 射线参数 t 的下界
};

// 正交相机：平行射线，由图像平面上的采样点决定射线起点
class OrthographicCamera : public Camera {

public:
  OrthographicCamera(Vec3f center, Vec3f direction, Vec3f up, float size);
  virtual Ray generateRay(Vec2f point);
  virtual float getTMin() const;

private:
  Vec3f center;       // 图像平面中心
  Vec3f direction;    // 射线方向
  Vec3f up;           // 图像平面竖直方向
  Vec3f horizontal;   // 图像平面水平方向
  float size;         // 图像平面边长
};

// 透视相机：射线从 center 发出，穿过虚拟成像平面上的采样点
class PerspectiveCamera : public Camera {

public:
  PerspectiveCamera(Vec3f center, Vec3f direction, Vec3f up, float angle);
  virtual Ray generateRay(Vec2f point);
  virtual float getTMin() const;

private:
  Vec3f center;       // 相机位置（射线起点）
  Vec3f direction;    // 视线方向
  Vec3f up;           // 成像平面竖直方向
  Vec3f horizontal;   // 成像平面水平方向
  float halfHeight;   // 成像平面半高
};

#endif
