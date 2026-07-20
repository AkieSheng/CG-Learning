#include "transform.h"
#include <cmath>
#include "grid.h"
#include "boundingbox.h"

Transform::Transform(Matrix &m, Object3D *o)
    : matrix(m), object(o)
{
  matrix.Inverse(inverseMatrix);
  inverseMatrix.Transpose();

  BoundingBox *childBox = object->getBoundingBox();

  if (childBox != nullptr)
  {
    Vec3f cmin = childBox->getMin();
    Vec3f cmax = childBox->getMax();
    Vec3f corners[8] = {
      Vec3f(cmin.x(), cmin.y(), cmin.z()),
      Vec3f(cmax.x(), cmin.y(), cmin.z()),
      Vec3f(cmin.x(), cmax.y(), cmin.z()),
      Vec3f(cmax.x(), cmax.y(), cmin.z()),
      Vec3f(cmin.x(), cmin.y(), cmax.z()),
      Vec3f(cmax.x(), cmin.y(), cmax.z()),
      Vec3f(cmin.x(), cmax.y(), cmax.z()),
      Vec3f(cmax.x(), cmax.y(), cmax.z())
    };

    matrix.Transform(corners[0]);
    Vec3f worldMin = corners[0];
    Vec3f worldMax = corners[0];
    for (int i = 1; i < 8; i++) {
      matrix.Transform(corners[i]);

      worldMin = Vec3f(::fminf(worldMin.x(), corners[i].x()),
                       ::fminf(worldMin.y(), corners[i].y()),
                       ::fminf(worldMin.z(), corners[i].z()));

      worldMax = Vec3f(::fmaxf(worldMax.x(), corners[i].x()),
                       ::fmaxf(worldMax.y(), corners[i].y()),
                       ::fmaxf(worldMax.z(), corners[i].z()));
    }
    bbox = new BoundingBox(worldMin, worldMax);
  }
}

Transform::~Transform()
{
  delete object;
}

void Transform::insertIntoGrid(Grid *g, Matrix *m)
{
  Matrix combined = matrix;
  if (m != nullptr)
    combined = (*m) * matrix;
  object->insertIntoGrid(g, &combined);
}

bool Transform::intersect(Ray const&r, Hit &h, float tmin)
{
  Matrix objectMatrix;
  matrix.Inverse(objectMatrix);

  Vec3f origin = r.getOrigin();
  Vec3f direction = r.getDirection();
  objectMatrix.Transform(origin);
  objectMatrix.TransformDirection(direction);

  Ray localRay(origin, direction);
  Hit localHit(h);
  if (!object->intersect(localRay, localHit, tmin))
    return false;

  Vec3f normal = localHit.getNormal();
  inverseMatrix.TransformDirection(normal);
  normal.Normalize();

  h.set(localHit.getT(), localHit.getMaterial(), normal, r);
  return true;
}

bool Transform::intersectShadow(Ray const&r, float tmin, float tmax, float &t,
                                Material **outMaterial)
{
  Matrix objectMatrix;
  matrix.Inverse(objectMatrix);

  Vec3f origin = r.getOrigin();
  Vec3f direction = r.getDirection();
  objectMatrix.Transform(origin);
  objectMatrix.TransformDirection(direction);

  Ray localRay(origin, direction);
  return object->intersectShadow(localRay, tmin, tmax, t, outMaterial);
}
