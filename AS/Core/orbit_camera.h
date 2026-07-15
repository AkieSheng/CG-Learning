#ifndef _ORBIT_CAMERA_H_
#define _ORBIT_CAMERA_H_

#include "vectors.h"
#include "matrix.h"

// 轨道相机
class OrbitCamera {
public:
  OrbitCamera();

  void setTarget(const Vec3f &t);
  void setDistance(float d);
  void setAspect(float aspect);
  void setFovY(float degrees);

  void rotate(float deltaYaw, float deltaPitch);  // 绕Y轴旋转
  void pan(float deltaX, float deltaY);  // 平移
  void zoom(float delta);  // 缩放

  void updateMatrices();

  const Matrix &getViewMatrix() const { return viewMatrix; }
  const Matrix &getProjectionMatrix() const { return projectionMatrix; }
  Vec3f getPosition() const { return position; }
  Vec3f getTarget() const { return target; }
  float getDistance() const { return distance; }
  Vec3f getFront() const;  // 获取视线方向
  Vec3f getUp() const { return worldUp; }

  void frameBounds(const Vec3f &bmin, const Vec3f &bmax);  // 根据模型包围盒自动设置观察距离

private:
  void updatePosition();

  Vec3f target;
  Vec3f worldUp;
  float distance;
  float yaw;  // 绕Y轴旋转角
  float pitch;  // 绕X轴旋转角
  float fovY;  // 垂直FOV
  float aspect;  // 宽高比

  Vec3f position;
  Matrix viewMatrix;  // 视图矩阵
  Matrix projectionMatrix;  // 投影矩阵
};

#endif
