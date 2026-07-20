#include "camera.h"
#include <math.h>

static Vec3f getScreenUp(Vec3f const&direction, Vec3f const&up) {
  Vec3f screenUp = up;
  screenUp -= direction * screenUp.Dot3(direction);
  screenUp.Normalize();
  return screenUp;
}

void OrthographicCamera::updateHorizontal() {
  direction.Normalize();
  Vec3f screenUp = getScreenUp(direction, up);
  Vec3f::Cross3(horizontal, direction, screenUp);
  horizontal.Normalize();
}

OrthographicCamera::OrthographicCamera(Vec3f c, Vec3f dir, Vec3f up_vec, float s) {
  center = c;
  size = s;
  direction = dir;
  up = up_vec;
  up.Normalize();
  updateHorizontal();
}

Ray OrthographicCamera::generateRay(Vec2f point) {
  float u = point.x();
  float v = point.y();
  Vec3f screenUp = getScreenUp(direction, up);
  Vec3f origin = center + (v - 0.5f) * size * screenUp
      + (u - 0.5f) * size * horizontal;

  return Ray(origin, direction);
}

float OrthographicCamera::getTMin() const {
  return -1.0e30f;
}

PerspectiveCamera::PerspectiveCamera(Vec3f c, Vec3f dir, Vec3f up_vec, float ang) {
  center = c;
  direction = dir;
  up = up_vec;
  up.Normalize();
  angle = ang;
  halfHeight = ::tanf(angle * 0.5f);
  updateHorizontal();
}

Ray PerspectiveCamera::generateRay(Vec2f point) {
  float u = point.x();
  float v = point.y();
  Vec3f screenUp = getScreenUp(direction, up);
  Vec3f screenPoint = center + direction
      + (v - 0.5f) * 2.0f * halfHeight * screenUp
      + (u - 0.5f) * 2.0f * halfHeight * horizontal;
  Vec3f rayDir = screenPoint - center;
  rayDir.Normalize();
  return Ray(center, rayDir);
}

float PerspectiveCamera::getTMin() const {
  return 1e-4f;
}

void PerspectiveCamera::updateHorizontal() {
  direction.Normalize();
  Vec3f screenUp = getScreenUp(direction, up);
  Vec3f::Cross3(horizontal, direction, screenUp);
  horizontal.Normalize();
}
