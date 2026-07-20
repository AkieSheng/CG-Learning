#include "object3d.h"
#include "boundingbox.h"
#include "grid.h"
#include "matrix.h"

Object3D::~Object3D()
{
  delete bbox;
}

void Object3D::insertIntoGrid(Grid *g, Matrix *m)
{
  if (g == nullptr || bbox == nullptr)
    return;
  g->insertObjectInBBox(bbox, this, m);
}
