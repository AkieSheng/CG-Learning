#include "object3d.h"
#include "boundingbox.h"
#include "grid.h"
#include "matrix.h"

#include <cstdio>

Object3D::~Object3D() {
  delete bbox;
}


auto Object3D::insertIntoGrid(Grid *g, Matrix *m) -> void {
  if (g == nullptr || bbox == nullptr)
    return;
  g->insertObjectInBBox(bbox, this, m);
}


auto Object3D::debugPrintBoundingBox(int depth)const -> void {
  for (int i = 0; i < depth; i++)
    ::printf("  ");
  if (bbox == nullptr)
    ::printf("Object3D: nullptr bounding box\n");
  else {
    ::printf("Object3D: ");
    bbox->Print();
  }
}
