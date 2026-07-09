#include "gl_headers.h"
#include "camera.h"
#include "matrix.h"
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

// 正交投影
void OrthographicCamera::glInit(int w, int h) {
  glMatrixMode(GL_PROJECTION);  // 设置投影矩阵
  glLoadIdentity();  // 加载单位矩阵
  if (w > h)  // 窄边裁剪
    glOrtho(-size / 2.0, size / 2.0,
            -size * (float)h / (float)w / 2.0,
            size * (float)h / (float)w / 2.0, 0.5, 40.0);
  else  // 窄边裁剪
    glOrtho(-size * (float)w / (float)h / 2.0,
            size * (float)w / (float)h / 2.0,
            -size / 2.0, size / 2.0, 0.5, 40.0);
}

void OrthographicCamera::glPlaceCamera(void) {
  gluLookAt(center.x(), center.y(), center.z(),  // 设置模型视图矩阵
            center.x() + direction.x(),
            center.y() + direction.y(),
            center.z() + direction.z(),
            up.x(), up.y(), up.z());
}

void OrthographicCamera::dollyCamera(float dist) {
  center += direction * dist;  // 沿视线方向移动 center
}

void OrthographicCamera::truckCamera(float dx, float dy) {
  Vec3f horizontalAxis;
  Vec3f::Cross3(horizontalAxis, direction, up);  // horizontalAxis = direction × up
  horizontalAxis.Normalize();
  Vec3f screenUp;
  Vec3f::Cross3(screenUp, horizontalAxis, direction);  // screenUp = horizontalAxis × direction
  center += horizontalAxis * dx + screenUp * dy;  // 在垂直于 direction 的平面内平移 center
  updateHorizontal();
}

void OrthographicCamera::rotateCamera(float rx, float ry) {
  Vec3f horizontalAxis;
  Vec3f::Cross3(horizontalAxis, direction, up);
  horizontalAxis.Normalize();

  // 限制俯仰角
  float tiltAngle = acosf(up.Dot3(direction));  // up 与 direction 的夹角（弧度）
  if (tiltAngle - ry > 3.13f)
    ry = tiltAngle - 3.13f;
  else if (tiltAngle - ry < 0.01f)
    ry = tiltAngle - 0.01f;

  Matrix rotMat = Matrix::MakeAxisRotation(up, rx);  // 绕 up 轴旋转 rx 弧度
  rotMat *= Matrix::MakeAxisRotation(horizontalAxis, ry);  // 绕 horizontalAxis 轴旋转 ry 弧度
  rotMat.Transform(center);  // 旋转 center
  rotMat.TransformDirection(direction);  // 旋转 direction
  direction.Normalize();
  updateHorizontal();
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

// 透视投影，视角为 angle，宽高比为 w/h，近截面为 0.5，远截面为 40.0
void PerspectiveCamera::glInit(int w, int h) {
  glMatrixMode(GL_PROJECTION);  // 设置投影矩阵
  glLoadIdentity();
  gluPerspective(angle * 180.0 / 3.14159, (float)w / float(h), 0.5, 40.0);  // 透视投影
}

// 设置模型视图矩阵
void PerspectiveCamera::glPlaceCamera(void) {
  gluLookAt(center.x(), center.y(), center.z(),
            center.x() + direction.x(),
            center.y() + direction.y(),
            center.z() + direction.z(),
            up.x(), up.y(), up.z());
}

void PerspectiveCamera::dollyCamera(float dist) {
  center += direction * dist;  // 沿视线方向移动 center
}

void PerspectiveCamera::truckCamera(float dx, float dy) {
  Vec3f horizontalAxis;
  Vec3f::Cross3(horizontalAxis, direction, up);  // horizontalAxis = direction × up
  horizontalAxis.Normalize();
  Vec3f screenUp;
  Vec3f::Cross3(screenUp, horizontalAxis, direction);  // screenUp = horizontalAxis × direction
  center += horizontalAxis * dx + screenUp * dy;  // 在垂直于 direction 的平面内平移 center
  updateHorizontal();
}

void PerspectiveCamera::rotateCamera(float rx, float ry) {
  Vec3f horizontalAxis;
  Vec3f::Cross3(horizontalAxis, direction, up);  // horizontalAxis = direction × up
  horizontalAxis.Normalize();

  float tiltAngle = acosf(up.Dot3(direction));  // up 与 direction 的夹角（弧度）
  if (tiltAngle - ry > 3.13f)
    ry = tiltAngle - 3.13f;
  else if (tiltAngle - ry < 0.01f)
    ry = tiltAngle - 0.01f;

  Matrix rotMat = Matrix::MakeAxisRotation(up, rx);  // 绕 up 轴旋转 rx 弧度
  rotMat *= Matrix::MakeAxisRotation(horizontalAxis, ry);  // 绕 horizontalAxis 轴旋转 ry 弧度
  rotMat.Transform(center);  // 旋转 center
  rotMat.TransformDirection(direction);  // 旋转 direction
  direction.Normalize();
  updateHorizontal();
}
