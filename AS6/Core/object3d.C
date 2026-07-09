#include "object3d.h"
#include "boundingbox.h"
#include "grid.h"
#include "matrix.h"

#include <stdio.h>

Object3D::~Object3D() {
  delete bbox;
}

// 将图元栅格化到体素网格
void Object3D::insertIntoGrid(Grid *g, Matrix *m) {
  if (g == NULL || bbox == NULL)
    return;
  g->insertObjectInBBox(bbox, this, m);
}

// [DEBUG] 打印包围盒
void Object3D::debugPrintBoundingBox(int depth) const {
  for (int i = 0; i < depth; i++)
    printf("  ");
  if (bbox == NULL)
    printf("Object3D: NULL bounding box\n");
  else {
    printf("Object3D: ");
    bbox->Print();
  }
}
