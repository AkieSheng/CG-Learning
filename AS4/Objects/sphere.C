#include "sphere.h"
#include "gl_options.h"
#include "gl_headers.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

auto Sphere::intersect(Ray const& r, Hit& h, float tmin) -> bool {
  auto oc = r.getOrigin() - center;
  auto dir = r.getDirection();
  auto a = dir.Dot3(dir);
  auto b = 2.0f * oc.Dot3(dir);
  auto c = oc.Dot3(oc) - radius * radius;
  auto discriminant = b * b - 4.0f * a * c;
  if (discriminant < 0.0f) {
    return false;
  }

  auto sqrt_disc = ::sqrtf(discriminant);
  auto hit = false;
  auto t = (-b - sqrt_disc) / (2.0f * a);
  if (t >= tmin && t < h.getT()) {
    auto normal = r.pointAtParameter(t) - center;
    normal.Normalize();
    h.set(t, material, normal, r);
    hit = true;
  }
  t = (-b + sqrt_disc) / (2.0f * a);
  if (t >= tmin && t < h.getT()) {
    auto normal = r.pointAtParameter(t) - center;
    normal.Normalize();
    h.set(t, material, normal, r);
    hit = true;
  }
  return hit;
}

auto Sphere::intersectShadow(Ray const& r, float tmin, float tmax, float& t,
                             Material** outMaterial) -> bool {
  auto oc = r.getOrigin() - center;
  auto dir = r.getDirection();
  auto a = dir.Dot3(dir);
  auto b = 2.0f * oc.Dot3(dir);
  auto c = oc.Dot3(oc) - radius * radius;
  auto discriminant = b * b - 4.0f * a * c;
  if (discriminant < 0.0f) {
    return false;
  }

  auto sqrt_disc = ::sqrtf(discriminant);
  auto tNear = (-b - sqrt_disc) / (2.0f * a);
  auto tFar = (-b + sqrt_disc) / (2.0f * a);

  if (outMaterial == nullptr) {
    if ((tNear >= tmin && tNear <= tmax) || (tFar >= tmin && tFar <= tmax)) {
      t = (tNear >= tmin && tNear <= tmax) ? tNear : tFar;
      return true;
    }
    return false;
  }

  auto hit = false;
  auto bestT = tmax;
  if (tNear >= tmin && tNear < bestT) {
    bestT = tNear;
    hit = true;
  }
  if (tFar >= tmin && tFar < bestT) {
    bestT = tFar;
    hit = true;
  }
  if (hit) {
    t = bestT;
    *outMaterial = material;
  }
  return hit;
}

static auto spherePoint(Vec3f const& center, float radius, float theta,
                        float phi) -> Vec3f {
  auto cosPhi = ::cosf(phi);
  Vec3f p(cosPhi * ::cosf(theta), ::sinf(phi), cosPhi * ::sinf(theta));
  return center + p * radius;
}

auto Sphere::paint() const -> void {
  if (material != nullptr) {
    material->glSetMaterial();
  }

  auto thetaSteps = tessellation_theta;
  auto phiSteps = tessellation_phi;
  if (thetaSteps < 2) {
    thetaSteps = 2;
  }
  if (phiSteps < 2) {
    phiSteps = 2;
  }
  auto dTheta = 2.0f * static_cast<float>(M_PI) / thetaSteps;
  auto dPhi = static_cast<float>(M_PI) / phiSteps;

  ::glBegin(GL_QUADS);
  for (auto iPhi = 0; iPhi < phiSteps; iPhi++) {
    auto phi0 = -0.5f * static_cast<float>(M_PI) + iPhi * dPhi;
    auto phi1 = phi0 + dPhi;
    for (auto iTheta = 0; iTheta < thetaSteps; iTheta++) {
      auto theta0 = iTheta * dTheta;
      auto theta1 = theta0 + dTheta;

      auto v00 = spherePoint(center, radius, theta0, phi0);
      auto v01 = spherePoint(center, radius, theta1, phi0);
      auto v11 = spherePoint(center, radius, theta1, phi1);
      auto v10 = spherePoint(center, radius, theta0, phi1);

      if (gouraud_shading) {
        auto n00 = v00 - center;
        n00.Normalize();
        auto n01 = v01 - center;
        n01.Normalize();
        auto n11 = v11 - center;
        n11.Normalize();
        auto n10 = v10 - center;
        n10.Normalize();

        ::glNormal3f(n00.x(), n00.y(), n00.z());
        ::glVertex3f(v00.x(), v00.y(), v00.z());
        ::glNormal3f(n01.x(), n01.y(), n01.z());
        ::glVertex3f(v01.x(), v01.y(), v01.z());
        ::glNormal3f(n11.x(), n11.y(), n11.z());
        ::glVertex3f(v11.x(), v11.y(), v11.z());
        ::glNormal3f(n10.x(), n10.y(), n10.z());
        ::glVertex3f(v10.x(), v10.y(), v10.z());
      } else {
        auto edge1 = v01 - v00;
        auto edge2 = v10 - v00;
        Vec3f faceNormal;
        Vec3f::Cross3(faceNormal, edge1, edge2);
        faceNormal.Normalize();

        ::glNormal3f(faceNormal.x(), faceNormal.y(), faceNormal.z());
        ::glVertex3f(v00.x(), v00.y(), v00.z());
        ::glVertex3f(v01.x(), v01.y(), v01.z());
        ::glVertex3f(v11.x(), v11.y(), v11.z());
        ::glVertex3f(v10.x(), v10.y(), v10.z());
      }
    }
  }
  ::glEnd();
}
