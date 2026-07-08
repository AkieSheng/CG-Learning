#include "camera.h"
#include <math.h>

// 建立相机正交基（direction、up、horizontal 两两垂直）
static void buildCameraBasis(Vec3f &direction, Vec3f &up, Vec3f &horizontal) {
  direction.Normalize();

  up -= direction * up.Dot3(direction);  // 去掉 up 在 direction 上的分量
  up.Normalize();

  Vec3f::Cross3(horizontal, direction, up);  // horizontal = direction × up
  horizontal.Normalize();
}

// 构造正交相机
OrthographicCamera::OrthographicCamera(Vec3f c, Vec3f dir, Vec3f up_vec, float s) {
  center = c;
  size = s;
  direction = dir;
  up = up_vec;
  buildCameraBasis(direction, up, horizontal);
}

// 生成射线
Ray OrthographicCamera::generateRay(Vec2f point) {
  float u = point.x();  // 屏幕坐标 x
  float v = point.y();  // 屏幕坐标 y
  Vec3f origin = center + (v - 0.5f) * size * up + (u - 0.5f) * size * horizontal;
  // 对应 center - size*up/2 - size*horizontal/2 到 center + size*up/2 + size*horizontal/2
  return Ray(origin, direction);
}

float OrthographicCamera::getTMin() const {
  return -1.0e30f;
}

// 构造透视相机
PerspectiveCamera::PerspectiveCamera(Vec3f c, Vec3f dir, Vec3f up_vec, float angle) {
  center = c;
  direction = dir;
  up = up_vec;
  buildCameraBasis(direction, up, horizontal);
  // 在距离为 1 的虚拟成像平面上，半高 = tan(fov/2)
  halfHeight = tanf(angle * 0.5f);
}

// 生成射线：在虚拟成像平面上插值，方向 = normalize(screenPoint - center)
// 参考 "virtual screen" 方法
Ray PerspectiveCamera::generateRay(Vec2f point) {
  float u = point.x();
  float v = point.y();
  // 成像平面放在 center + direction 处
  Vec3f screenPoint = center + direction
      + (v - 0.5f) * 2.0f * halfHeight * up
      + (u - 0.5f) * 2.0f * halfHeight * horizontal;
  Vec3f rayDir = screenPoint - center;
  rayDir.Normalize();
  return Ray(center, rayDir);
}

float PerspectiveCamera::getTMin() const {
  return 1e-4f;  // 小正数，避免自相交
}
