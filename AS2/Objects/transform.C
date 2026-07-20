#include "transform.h"

auto Transform::intersect(Ray const& r, Hit& h, float tmin) -> bool
{
  Matrix objectMatrix;
  matrix.Inverse(objectMatrix);

  auto origin = r.getOrigin();
  auto direction = r.getDirection();
  objectMatrix.Transform(origin);
  objectMatrix.TransformDirection(direction);

  Ray localRay(origin, direction);
  Hit localHit(h);
  if (!object->intersect(localRay, localHit, tmin))
    return false;

  auto normal = localHit.getNormal();
  inverseMatrix.TransformDirection(normal);
  normal.Normalize();

  h.set(localHit.getT(), localHit.getMaterial(), normal, r);
  return true;
}
