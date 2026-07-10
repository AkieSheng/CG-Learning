#include "object3d.h"
#include "boundingbox.h"
#include "grid.h"
#include "matrix.h"

Object3D::~Object3D() {
  delete bbox;
}

// 将图元栅格化到体素网格
void Object3D::insertIntoGrid(Grid *g, Matrix *m) {
  if (g == NULL || bbox == NULL)
    return;
  g->insertObjectInBBox(bbox, this, m);
}
