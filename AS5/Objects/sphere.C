#include "sphere.h"
#include "grid.h"
#include "boundingbox.h"
#include "matrix.h"
#include "gl_options.h"
#include "gl_headers.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Sphere::Sphere(Vec3f center, float radius, Material *m)
    : center(center), radius(radius) {
  material = m;
  Vec3f rVec(radius, radius, radius);
  bbox = new BoundingBox(center - rVec, center + rVec);
}

auto Sphere::insertIntoGrid(Grid *g, Matrix *m) -> void {
  if (g == nullptr)
    return;

  float halfDiag = g->getVoxelHalfDiagonal();
  float testRadius = radius + halfDiag;

  Matrix inv;
  if (m != nullptr)
    m->Inverse(inv);

  Vec3f rmin = bbox->getMin();
  Vec3f rmax = bbox->getMax();
  if (m != nullptr) {
    Vec3f corners[8] = {
      Vec3f(rmin.x(), rmin.y(), rmin.z()),
      Vec3f(rmax.x(), rmin.y(), rmin.z()),
      Vec3f(rmin.x(), rmax.y(), rmin.z()),
      Vec3f(rmax.x(), rmax.y(), rmin.z()),
      Vec3f(rmin.x(), rmin.y(), rmax.z()),
      Vec3f(rmax.x(), rmin.y(), rmax.z()),
      Vec3f(rmin.x(), rmax.y(), rmax.z()),
      Vec3f(rmax.x(), rmax.y(), rmax.z())
    };
    m->Transform(corners[0]);
    rmin = corners[0];
    rmax = corners[0];
    for (int i = 1; i < 8; i++) {
      m->Transform(corners[i]);
      rmin = Vec3f(::fminf(rmin.x(), corners[i].x()),
                   ::fminf(rmin.y(), corners[i].y()),
                   ::fminf(rmin.z(), corners[i].z()));
      rmax = Vec3f(::fmaxf(rmax.x(), corners[i].x()),
                   ::fmaxf(rmax.y(), corners[i].y()),
                   ::fmaxf(rmax.z(), corners[i].z()));
    }
  }

  Vec3f margin(halfDiag, halfDiag, halfDiag);
  rmin = rmin - margin;
  rmax = rmax + margin;

  Vec3f bbMin = g->getBoundingBox()->getMin();
  Vec3f bbMax = g->getBoundingBox()->getMax();
  float dx = (bbMax.x() - bbMin.x()) / g->getNX();
  float dy = (bbMax.y() - bbMin.y()) / g->getNY();
  float dz = (bbMax.z() - bbMin.z()) / g->getNZ();
  int i0 = (int)((rmin.x() - bbMin.x()) / dx);
  int i1 = (int)((rmax.x() - bbMin.x()) / dx);
  int j0 = (int)((rmin.y() - bbMin.y()) / dy);
  int j1 = (int)((rmax.y() - bbMin.y()) / dy);
  int k0 = (int)((rmin.z() - bbMin.z()) / dz);
  int k1 = (int)((rmax.z() - bbMin.z()) / dz);
  if (i0 < 0) i0 = 0;
  if (j0 < 0) j0 = 0;
  if (k0 < 0) k0 = 0;
  if (i0 >= g->getNX()) i0 = g->getNX() - 1;
  if (j0 >= g->getNY()) j0 = g->getNY() - 1;
  if (k0 >= g->getNZ()) k0 = g->getNZ() - 1;
  if (i1 >= g->getNX()) i1 = g->getNX() - 1;
  if (j1 >= g->getNY()) j1 = g->getNY() - 1;
  if (k1 >= g->getNZ()) k1 = g->getNZ() - 1;

  for (int i = i0; i <= i1; i++) {
    for (int j = j0; j <= j1; j++) {
      for (int k = k0; k <= k1; k++) {
        Vec3f voxelCenter = g->getVoxelCenter(i, j, k);
        if (m != nullptr)
          inv.Transform(voxelCenter);
        if ((voxelCenter - center).Length() <= testRadius)
          g->insertObject(i, j, k, this);
      }
    }
  }
}

auto Sphere::debugPrintBoundingBox(int depth) const -> void {
  for (int i = 0; i < depth; i++)
    ::printf("  ");
  ::printf("Sphere: ");
  if (bbox == nullptr)
    ::printf("nullptr bounding box\n");
  else
    bbox->Print();
}

auto Sphere::intersect(Ray const&r, Hit &h, float tmin) -> bool {
  Vec3f oc = r.getOrigin() - center;
  Vec3f dir = r.getDirection();
  float a = dir.Dot3(dir);
  float b = 2.0f * oc.Dot3(dir);
  float c = oc.Dot3(oc) - radius * radius;
  float discriminant = b * b - 4.0f * a * c;
  if (discriminant < 0.0f)
    return false;

  float sqrt_disc = ::sqrtf(discriminant);
  bool hit = false;
  float t = (-b - sqrt_disc) / (2.0f * a);
  if (t >= tmin && t < h.getT()) {
    Vec3f normal = r.pointAtParameter(t) - center;
    normal.Normalize();
    h.set(t, material, normal, r);
    hit = true;
  }
  t = (-b + sqrt_disc) / (2.0f * a);
  if (t >= tmin && t < h.getT()) {
    Vec3f normal = r.pointAtParameter(t) - center;
    normal.Normalize();
    h.set(t, material, normal, r);
    hit = true;
  }
  return hit;
}

auto Sphere::intersectShadow(Ray const&r, float tmin, float tmax, float &t,
                             Material **outMaterial) -> bool {
  Vec3f oc = r.getOrigin() - center;
  Vec3f dir = r.getDirection();
  float a = dir.Dot3(dir);
  float b = 2.0f * oc.Dot3(dir);
  float c = oc.Dot3(oc) - radius * radius;
  float discriminant = b * b - 4.0f * a * c;
  if (discriminant < 0.0f)
    return false;

  float sqrt_disc = ::sqrtf(discriminant);
  float tNear = (-b - sqrt_disc) / (2.0f * a);
  float tFar = (-b + sqrt_disc) / (2.0f * a);

  if (outMaterial == nullptr) {
    if ((tNear >= tmin && tNear <= tmax) || (tFar >= tmin && tFar <= tmax)) {
      t = (tNear >= tmin && tNear <= tmax) ? tNear : tFar;
      return true;
    }
    return false;
  }

  bool hit = false;
  float bestT = tmax;
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

static auto spherePoint(Vec3f const&center, float radius,
                         float theta, float phi) -> Vec3f {
  float cosPhi = ::cosf(phi);
  Vec3f p(cosPhi * ::cosf(theta), ::sinf(phi), cosPhi * ::sinf(theta));
  return center + p * radius;
}

auto Sphere::paint() const -> void {
  if (material != nullptr)
    material->glSetMaterial();

  int thetaSteps = tessellation_theta;
  int phiSteps = tessellation_phi;
  if (thetaSteps < 2) thetaSteps = 2;
  if (phiSteps < 2) phiSteps = 2;
  float dTheta = 2.0f * (float)M_PI / thetaSteps;
  float dPhi = (float)M_PI / phiSteps;

  glBegin(GL_QUADS);
  for (int iPhi = 0; iPhi < phiSteps; iPhi++) {
    float phi0 = -0.5f * (float)M_PI + iPhi * dPhi;
    float phi1 = phi0 + dPhi;
    for (int iTheta = 0; iTheta < thetaSteps; iTheta++) {
      float theta0 = iTheta * dTheta;
      float theta1 = theta0 + dTheta;

      Vec3f v00 = spherePoint(center, radius, theta0, phi0);
      Vec3f v01 = spherePoint(center, radius, theta1, phi0);
      Vec3f v11 = spherePoint(center, radius, theta1, phi1);
      Vec3f v10 = spherePoint(center, radius, theta0, phi1);

      if (gouraud_shading) {
        Vec3f n00 = v00 - center; n00.Normalize();
        Vec3f n01 = v01 - center; n01.Normalize();
        Vec3f n11 = v11 - center; n11.Normalize();
        Vec3f n10 = v10 - center; n10.Normalize();

        glNormal3f(n00.x(), n00.y(), n00.z());
        glVertex3f(v00.x(), v00.y(), v00.z());
        glNormal3f(n01.x(), n01.y(), n01.z());
        glVertex3f(v01.x(), v01.y(), v01.z());
        glNormal3f(n11.x(), n11.y(), n11.z());
        glVertex3f(v11.x(), v11.y(), v11.z());
        glNormal3f(n10.x(), n10.y(), n10.z());
        glVertex3f(v10.x(), v10.y(), v10.z());
      } else {
        Vec3f edge1 = v01 - v00;
        Vec3f edge2 = v10 - v00;
        Vec3f faceNormal;
        Vec3f::Cross3(faceNormal, edge1, edge2);
        faceNormal.Normalize();

        glNormal3f(faceNormal.x(), faceNormal.y(), faceNormal.z());
        glVertex3f(v00.x(), v00.y(), v00.z());
        glVertex3f(v01.x(), v01.y(), v01.z());
        glVertex3f(v11.x(), v11.y(), v11.z());
        glVertex3f(v10.x(), v10.y(), v10.z());
      }
    }
  }
  glEnd();
}
