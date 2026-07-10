#include "camera.h"
#include <math.h>

// 计算 screen up
static Vec3f getScreenUp(const Vec3f &direction, const Vec3f &up) {
  Vec3f screenUp = up;
  screenUp -= direction * screenUp.Dot3(direction);  // 去掉 up 在 direction 上的分量
  screenUp.Normalize();  // 归一化
  return screenUp;
}

// 建立相机正交基并重算 horizontal
void OrthographicCamera::updateHorizontal() {
  direction.Normalize();
  Vec3f screenUp = getScreenUp(direction, up);
  Vec3f::Cross3(horizontal, direction, screenUp);  // horizontal = direction × screenUp
  horizontal.Normalize();
}

// 构造正交相机
OrthographicCamera::OrthographicCamera(Vec3f c, Vec3f dir, Vec3f up_vec, float s) {
  center = c;
  size = s;
  direction = dir;
  up = up_vec;
  up.Normalize();
  updateHorizontal();
}

// 生成射线：在成像平面上按 (u,v) 插值得到起点
Ray OrthographicCamera::generateRay(Vec2f point) {
  float u = point.x();  // 屏幕坐标 x
  float v = point.y();  // 屏幕坐标 y
  Vec3f screenUp = getScreenUp(direction, up);
  Vec3f origin = center + (v - 0.5f) * size * screenUp
      + (u - 0.5f) * size * horizontal;
  // 对应 center - size*screenUp/2 - size*horizontal/2 到 center + ...
  return Ray(origin, direction);
}

float OrthographicCamera::getTMin() const {
  return -1.0e30f;
}

// 构造透视相机
PerspectiveCamera::PerspectiveCamera(Vec3f c, Vec3f dir, Vec3f up_vec, float ang) {
  center = c;
  direction = dir;
  up = up_vec;
  up.Normalize();
  angle = ang;
  halfHeight = tanf(angle * 0.5f);  // 成像平面半高
  updateHorizontal();
}

// 生成射线
Ray PerspectiveCamera::generateRay(Vec2f point) {
  float u = point.x();
  float v = point.y();
  Vec3f screenUp = getScreenUp(direction, up);  // screenUp = up - direction * up.Dot3(direction)
  // 成像平面放在 center + direction 处，通过 screenUp 和 horizontal 插值得到成像平面上的点，然后计算射线方向
  // 参考 "virtual screen" 方法
  Vec3f screenPoint = center + direction
      + (v - 0.5f) * 2.0f * halfHeight * screenUp
      + (u - 0.5f) * 2.0f * halfHeight * horizontal;
  Vec3f rayDir = screenPoint - center;
  rayDir.Normalize();
  return Ray(center, rayDir);
}

float PerspectiveCamera::getTMin() const {
  return 1e-4f;  // 容差
}
 
void PerspectiveCamera::updateHorizontal() {
  direction.Normalize();
  Vec3f screenUp = getScreenUp(direction, up);
  Vec3f::Cross3(horizontal, direction, screenUp);  // horizontal = direction × screenUp
  horizontal.Normalize();
}
