#include "camera.h"

// 构造正交相机
OrthographicCamera::OrthographicCamera(Vec3f c, Vec3f dir, Vec3f up_vec, float s) {
  center = c;
  size = s;

  direction = dir;
  direction /= direction.Length();  // 归一化

  up = up_vec;
  up -= direction * up.Dot3(direction); // 投影到 direction 方向
  up /= up.Length();  // 归一化

  Vec3f::Cross3(horizontal, direction, up); // horizontal = direction × up，得到图像平面水平轴
  horizontal /= horizontal.Length(); // 归一化，得到图像平面水平轴的单位向量
}

// 生成射线
Ray OrthographicCamera::generateRay(Vec2f point) {
  float u = point.x();  // 屏幕坐标 x
  float v = point.y();  // 屏幕坐标 y
  Vec3f origin = center + (v - 0.5f) * size * up + (u - 0.5f) * size * horizontal; // 射线起点
  // 对应 center - size*up/2 - size*horizontal/2 到 center + size*up/2 + size*horizontal/2
  return Ray(origin, direction);
}

float OrthographicCamera::getTMin() const {
  return -1.0e30f; // tmin 取大负数，正交相机射线从无穷远进入场景
}
