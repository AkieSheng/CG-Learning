#include "orbit_camera.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

OrbitCamera::OrbitCamera()
  : target(0, 0, 0),
    worldUp(0, 1, 0),
    distance(5.0f),
    yaw(0.0f),
    pitch(20.0f),
    fovY(45.0f),
    aspect(1.0f) {
  viewMatrix.SetToIdentity();
  projectionMatrix.SetToIdentity();
  updatePosition();
  updateMatrices();
}

void OrbitCamera::setTarget(const Vec3f &t) {
  target = t;
  updatePosition();
  updateMatrices();
}

void OrbitCamera::setDistance(float d) {
  distance = d;
  if (distance < 0.1f) distance = 0.1f;
  updatePosition();
  updateMatrices();
}

void OrbitCamera::setAspect(float a) {
  aspect = a;
  updateMatrices();
}

void OrbitCamera::setFovY(float degrees) {
  fovY = degrees;
  updateMatrices();
}

void OrbitCamera::rotate(float deltaYaw, float deltaPitch) {
  yaw += deltaYaw;
  pitch += deltaPitch;
  if (pitch > 89.0f) pitch = 89.0f;
  if (pitch < -89.0f) pitch = -89.0f;
  updatePosition();
  updateMatrices();
}

void OrbitCamera::pan(float deltaX, float deltaY) {
  Vec3f front = getFront();
  Vec3f right, up;
  Vec3f::Cross3(right, front, worldUp);
  right.Normalize();
  Vec3f::Cross3(up, right, front);
  up.Normalize();
  target += right * deltaX + up * deltaY;
  updatePosition();
  updateMatrices();
}

void OrbitCamera::zoom(float delta) {
  setDistance(distance * (1.0f - delta));
}

void OrbitCamera::updatePosition() {
  float yawRad = yaw * (float)M_PI / 180.0f;
  float pitchRad = pitch * (float)M_PI / 180.0f;
  float x = distance * cosf(pitchRad) * sinf(yawRad);
  float y = distance * sinf(pitchRad);
  float z = distance * cosf(pitchRad) * cosf(yawRad);
  position = target + Vec3f(x, y, z);
}

Vec3f OrbitCamera::getFront() const {
  Vec3f f = target - position;
  f.Normalize();
  return f;
}

void OrbitCamera::updateMatrices() {
  Vec3f front = getFront();
  Vec3f right, up;
  Vec3f::Cross3(right, front, worldUp);
  right.Normalize();
  Vec3f::Cross3(up, right, front);
  up.Normalize();

  viewMatrix.SetToIdentity();
  viewMatrix.Set(0, 0, right.x());
  viewMatrix.Set(1, 0, right.y());
  viewMatrix.Set(2, 0, right.z());
  viewMatrix.Set(0, 1, up.x());
  viewMatrix.Set(1, 1, up.y());
  viewMatrix.Set(2, 1, up.z());
  viewMatrix.Set(0, 2, -front.x());
  viewMatrix.Set(1, 2, -front.y());
  viewMatrix.Set(2, 2, -front.z());
  // 平移必须写入列 3（data[0..2][3]），glGet() 上传给 OpenGL
  viewMatrix.Set(3, 0, -right.Dot3(position));
  viewMatrix.Set(3, 1, -up.Dot3(position));
  viewMatrix.Set(3, 2, front.Dot3(position));

  float fovRad = fovY * (float)M_PI / 180.0f;
  float tanHalf = tanf(fovRad * 0.5f);
  float nearZ = 0.1f;
  float farZ = 1000.0f;

  projectionMatrix.Clear();
  projectionMatrix.Set(0, 0, 1.0f / (aspect * tanHalf));
  projectionMatrix.Set(1, 1, 1.0f / tanHalf);
  projectionMatrix.Set(2, 2, -(farZ + nearZ) / (farZ - nearZ));
  projectionMatrix.Set(3, 2, -(2.0f * farZ * nearZ) / (farZ - nearZ));
  projectionMatrix.Set(2, 3, -1.0f);
}

void OrbitCamera::frameBounds(const Vec3f &bmin, const Vec3f &bmax) {
  Vec3f center = (bmin + bmax) * 0.5f;
  Vec3f extent = bmax - bmin;
  float radius = 0.5f * extent.x();
  if (0.5f * extent.y() > radius) radius = 0.5f * extent.y();
  if (0.5f * extent.z() > radius) radius = 0.5f * extent.z();
  if (radius < 0.001f) radius = 1.0f;

  yaw = 0.0f;
  pitch = 20.0f;
  setTarget(center);
  float fovRad = fovY * (float)M_PI / 180.0f;
  float dist = radius / tanf(fovRad * 0.5f);
  setDistance(dist * 1.8f);
}
