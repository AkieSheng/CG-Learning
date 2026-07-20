#include "orbit_camera.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

OrbitCamera::OrbitCamera() {
  viewMatrix.SetToIdentity();
  projectionMatrix.SetToIdentity();
  updatePosition();
  updateMatrices();
}

auto OrbitCamera::setTarget(Vec3f const& t) -> void {
  target = t;
  updatePosition();
  updateMatrices();
}

auto OrbitCamera::setDistance(float d) -> void {
  distance = d;
  if (distance < 0.1f) {
    distance = 0.1f;
  }
  updatePosition();
  updateMatrices();
}

auto OrbitCamera::setAspect(float a) -> void {
  aspect = a;
  updateMatrices();
}

auto OrbitCamera::setFovY(float degrees) -> void {
  fovY = degrees;
  updateMatrices();
}

auto OrbitCamera::rotate(float deltaYaw, float deltaPitch) -> void {
  yaw += deltaYaw;
  pitch += deltaPitch;
  if (pitch > 89.0f) {
    pitch = 89.0f;
  }
  if (pitch < -89.0f) {
    pitch = -89.0f;
  }
  updatePosition();
  updateMatrices();
}

auto OrbitCamera::pan(float deltaX, float deltaY) -> void {
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

auto OrbitCamera::zoom(float delta) -> void {
  setDistance(distance * (1.0f - delta));
}

auto OrbitCamera::updatePosition() -> void {
  float yawRad = yaw * static_cast<float>(M_PI) / 180.0f;
  float pitchRad = pitch * static_cast<float>(M_PI) / 180.0f;
  float x = distance * std::cos(pitchRad) * std::sin(yawRad);
  float y = distance * std::sin(pitchRad);
  float z = distance * std::cos(pitchRad) * std::cos(yawRad);
  position = target + Vec3f(x, y, z);
}

auto OrbitCamera::getFront() const -> Vec3f {
  Vec3f f = target - position;
  f.Normalize();
  return f;
}

auto OrbitCamera::updateMatrices() -> void {
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
  viewMatrix.Set(3, 0, -right.Dot3(position));
  viewMatrix.Set(3, 1, -up.Dot3(position));
  viewMatrix.Set(3, 2, front.Dot3(position));

  float fovRad = fovY * static_cast<float>(M_PI) / 180.0f;
  float tanHalf = std::tan(fovRad * 0.5f);
  nearZ = 0.1f;
  farZ = 1000.0f;
  if (distance > farZ * 0.45f) {
    nearZ = distance * 0.01f;
    if (nearZ < 0.1f) {
      nearZ = 0.1f;
    }
    farZ = distance * 10.0f;
  }

  projectionMatrix.Clear();
  projectionMatrix.Set(0, 0, 1.0f / (aspect * tanHalf));
  projectionMatrix.Set(1, 1, 1.0f / tanHalf);
  projectionMatrix.Set(2, 2, -(farZ + nearZ) / (farZ - nearZ));
  projectionMatrix.Set(3, 2, -(2.0f * farZ * nearZ) / (farZ - nearZ));
  projectionMatrix.Set(2, 3, -1.0f);
}

auto OrbitCamera::frameBounds(Vec3f const& bmin, Vec3f const& bmax) -> void {
  Vec3f center = (bmin + bmax) * 0.5f;
  Vec3f extent = bmax - bmin;
  float radius = 0.5f * extent.x();
  if (0.5f * extent.y() > radius) {
    radius = 0.5f * extent.y();
  }
  if (0.5f * extent.z() > radius) {
    radius = 0.5f * extent.z();
  }
  if (radius < 0.001f) {
    radius = 1.0f;
  }

  yaw = 0.0f;
  pitch = 20.0f;
  setTarget(center);
  float fovRad = fovY * static_cast<float>(M_PI) / 180.0f;
  float dist = radius / std::tan(fovRad * 0.5f);
  setDistance(dist * 1.8f);
}
