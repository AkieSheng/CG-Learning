#include "camera.h"

#include "matrix.h"
#include "gl_headers.h"

#include <cmath>


static auto getScreenUp(Vec3f const& direction, Vec3f const& up) -> Vec3f
{
  auto screenUp = up;
  screenUp -= direction * screenUp.Dot3(direction);
  screenUp.Normalize();
  return screenUp;
}

auto OrthographicCamera::updateHorizontal() -> void
{
  direction.Normalize();
  auto screenUp = getScreenUp(direction, up);
  Vec3f::Cross3(horizontal, direction, screenUp);
  horizontal.Normalize();
}

OrthographicCamera::OrthographicCamera(Vec3f c, Vec3f dir, Vec3f up_vec, float s)
{
  center = c;
  size = s;
  direction = dir;
  up = up_vec;
  up.Normalize();
  updateHorizontal();
}

auto OrthographicCamera::generateRay(Vec2f point) -> Ray
{
  auto u = point.x();
  auto v = point.y();
  auto screenUp = getScreenUp(direction, up);
  auto origin = center + (v - 0.5f) * size * screenUp
      + (u - 0.5f) * size * horizontal;
  return Ray(origin, direction);
}

auto OrthographicCamera::getTMin() const -> float
{
  return -1.0e30f;
}

auto OrthographicCamera::glInit(int w, int h) -> void
{
  ::glMatrixMode(GL_PROJECTION);
  ::glLoadIdentity();
  if (w > h) {
    ::glOrtho(-size / 2.0, size / 2.0,
              -size * static_cast<float>(h) / static_cast<float>(w) / 2.0,
              size * static_cast<float>(h) / static_cast<float>(w) / 2.0,
              0.5, 40.0);
  } else {
    ::glOrtho(-size * static_cast<float>(w) / static_cast<float>(h) / 2.0,
              size * static_cast<float>(w) / static_cast<float>(h) / 2.0,
              -size / 2.0, size / 2.0, 0.5, 40.0);
  }
}

auto OrthographicCamera::glPlaceCamera() -> void
{
  ::gluLookAt(center.x(), center.y(), center.z(),
              center.x() + direction.x(),
              center.y() + direction.y(),
              center.z() + direction.z(),
              up.x(), up.y(), up.z());
}

auto OrthographicCamera::dollyCamera(float dist) -> void
{
  center += direction * dist;
}

auto OrthographicCamera::truckCamera(float dx, float dy) -> void
{
  Vec3f horizontalAxis;
  Vec3f::Cross3(horizontalAxis, direction, up);
  horizontalAxis.Normalize();
  Vec3f screenUp;
  Vec3f::Cross3(screenUp, horizontalAxis, direction);
  center += horizontalAxis * dx + screenUp * dy;
  updateHorizontal();
}

auto OrthographicCamera::rotateCamera(float rx, float ry) -> void
{
  Vec3f horizontalAxis;
  Vec3f::Cross3(horizontalAxis, direction, up);
  horizontalAxis.Normalize();

  auto tiltAngle = ::acosf(up.Dot3(direction));
  if (tiltAngle - ry > 3.13f) {
    ry = tiltAngle - 3.13f;
  } else if (tiltAngle - ry < 0.01f) {
    ry = tiltAngle - 0.01f;
  }

  auto rotMat = Matrix::MakeAxisRotation(up, rx);
  rotMat *= Matrix::MakeAxisRotation(horizontalAxis, ry);
  rotMat.Transform(center);
  rotMat.TransformDirection(direction);
  direction.Normalize();
  updateHorizontal();
}

PerspectiveCamera::PerspectiveCamera(Vec3f c, Vec3f dir, Vec3f up_vec, float ang)
{
  center = c;
  direction = dir;
  up = up_vec;
  up.Normalize();
  angle = ang;
  halfHeight = ::tanf(angle * 0.5f);
  updateHorizontal();
}

auto PerspectiveCamera::generateRay(Vec2f point) -> Ray
{
  auto u = point.x();
  auto v = point.y();
  auto screenUp = getScreenUp(direction, up);
  auto screenPoint = center + direction
      + (v - 0.5f) * 2.0f * halfHeight * screenUp
      + (u - 0.5f) * 2.0f * halfHeight * horizontal;
  auto rayDir = screenPoint - center;
  rayDir.Normalize();
  return Ray(center, rayDir);
}

auto PerspectiveCamera::getTMin() const -> float
{
  return 1e-4f;
}

auto PerspectiveCamera::updateHorizontal() -> void
{
  direction.Normalize();
  auto screenUp = getScreenUp(direction, up);
  Vec3f::Cross3(horizontal, direction, screenUp);
  horizontal.Normalize();
}

auto PerspectiveCamera::glInit(int w, int h) -> void
{
  ::glMatrixMode(GL_PROJECTION);
  ::glLoadIdentity();
  ::gluPerspective(angle * 180.0 / 3.14159,
                   static_cast<float>(w) / static_cast<float>(h),
                   0.5, 40.0);
}

auto PerspectiveCamera::glPlaceCamera() -> void
{
  ::gluLookAt(center.x(), center.y(), center.z(),
              center.x() + direction.x(),
              center.y() + direction.y(),
              center.z() + direction.z(),
              up.x(), up.y(), up.z());
}

auto PerspectiveCamera::dollyCamera(float dist) -> void
{
  center += direction * dist;
}

auto PerspectiveCamera::truckCamera(float dx, float dy) -> void
{
  Vec3f horizontalAxis;
  Vec3f::Cross3(horizontalAxis, direction, up);
  horizontalAxis.Normalize();
  Vec3f screenUp;
  Vec3f::Cross3(screenUp, horizontalAxis, direction);
  center += horizontalAxis * dx + screenUp * dy;
  updateHorizontal();
}

auto PerspectiveCamera::rotateCamera(float rx, float ry) -> void
{
  Vec3f horizontalAxis;
  Vec3f::Cross3(horizontalAxis, direction, up);
  horizontalAxis.Normalize();

  auto tiltAngle = ::acosf(up.Dot3(direction));
  if (tiltAngle - ry > 3.13f) {
    ry = tiltAngle - 3.13f;
  } else if (tiltAngle - ry < 0.01f) {
    ry = tiltAngle - 0.01f;
  }

  auto rotMat = Matrix::MakeAxisRotation(up, rx);
  rotMat *= Matrix::MakeAxisRotation(horizontalAxis, ry);
  rotMat.Transform(center);
  rotMat.TransformDirection(direction);
  direction.Normalize();
  updateHorizontal();
}
