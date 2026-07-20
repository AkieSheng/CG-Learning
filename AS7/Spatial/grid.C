#include "grid.h"
#include "boundingbox.h"
#include "marchinginfo.h"
#include "material.h"
#include "ray.h"
#include "hit.h"
#include "matrix.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static const float GRID_EPSILON = 1.0e-6f;
static const float GRID_INF = 1.0e30f;

struct GridTransform : public Object3D {

public:
  GridTransform(Matrix const&m, Object3D *o) : matrix(m), object(o) {
    matrix.Inverse(inverseMatrix);
    inverseMatrix.Transpose();
  }

  virtual bool intersect(Ray const&r, Hit &h, float tmin) {
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

  virtual bool intersectShadow(Ray const&r, float tmin, float tmax, float &t,
                               Material **outMaterial) {
    Matrix objectMatrix;
    matrix.Inverse(objectMatrix);

    Vec3f origin = r.getOrigin();
    Vec3f direction = r.getDirection();
    objectMatrix.Transform(origin);
    objectMatrix.TransformDirection(direction);

    Ray localRay(origin, direction);
    return object->intersectShadow(localRay, tmin, tmax, t, outMaterial);
  }

private:
  Matrix matrix;
  Matrix inverseMatrix;
  Object3D *object;
};

static bool boxesOverlap(Vec3f const&amin, Vec3f const&amax,
                         Vec3f const&bmin, Vec3f const&bmax) {
  return amin.x() <= bmax.x() && amax.x() >= bmin.x() &&
         amin.y() <= bmax.y() && amax.y() >= bmin.y() &&
         amin.z() <= bmax.z() && amax.z() >= bmin.z();
}

Grid::Grid(BoundingBox *bb, int nx, int ny, int nz)
    : sceneBounds(nullptr), nx(nx), ny(ny), nz(nz),
      dx(0), dy(0), dz(0), cells(nullptr),
      infiniteObjects(nullptr), gridWrappers(nullptr) {

  assert(bb != nullptr);
  assert(nx > 0 && ny > 0 && nz > 0);

  Vec3f bbMin = bb->getMin();
  Vec3f bbMax = bb->getMax();
  sceneBounds = new BoundingBox(bbMin, bbMax);

  dx = (bbMax.x() - bbMin.x()) / nx;
  dy = (bbMax.y() - bbMin.y()) / ny;
  dz = (bbMax.z() - bbMin.z()) / nz;

  int numCells = nx * ny * nz;
  cells = new Object3DVector[numCells];
  infiniteObjects = new Object3DVector();
  gridWrappers = new Object3DVector();

  material = nullptr;
}

Grid::~Grid() {
  if (gridWrappers != nullptr) {
    for (int i = 0; i < gridWrappers->getNumObjects(); i++)
      delete gridWrappers->getObject(i);
    delete gridWrappers;
  }
  gridWrappers = nullptr;

  delete infiniteObjects;
  infiniteObjects = nullptr;

  material = nullptr;

  delete [] cells;
  cells = nullptr;
  delete sceneBounds;
  sceneBounds = nullptr;
}

int Grid::index(int i, int j, int k) const {
  assert(i >= 0 && i < nx);
  assert(j >= 0 && j < ny);
  assert(k >= 0 && k < nz);
  return i * ny * nz + j * nz + k;
}

bool Grid::inBounds(int i, int j, int k) const {
  return i >= 0 && i < nx && j >= 0 && j < ny && k >= 0 && k < nz;
}

void Grid::getVoxelBounds(int i, int j, int k, Vec3f &vmin, Vec3f &vmax) const {
  Vec3f bbMin = sceneBounds->getMin();
  vmin = Vec3f(bbMin.x() + i * dx,
               bbMin.y() + j * dy,
               bbMin.z() + k * dz);
  vmax = Vec3f(vmin.x() + dx, vmin.y() + dy, vmin.z() + dz);
}

Vec3f Grid::getVoxelCenter(int i, int j, int k) const {
  Vec3f vmin, vmax;
  getVoxelBounds(i, j, k, vmin, vmax);
  return (vmin + vmax) * 0.5f;
}

float Grid::getVoxelHalfDiagonal() const {
  return 0.5f * ::sqrtf(dx * dx + dy * dy + dz * dz);
}

Object3D *Grid::wrapForGrid(Object3D *obj, Matrix *m) {
  if (m == nullptr)
    return obj;
  GridTransform *wrapper = new GridTransform(*m, obj);
  gridWrappers->addObject(wrapper);
  return wrapper;
}

void Grid::addInfiniteObject(Object3D *obj) {
  assert(obj != nullptr);
  infiniteObjects->addObject(obj);
}

Object3DVector *Grid::getCell(int i, int j, int k) {
  return &cells[index(i, j, k)];
}

void Grid::insertObject(int i, int j, int k, Object3D *obj) {
  assert(obj != nullptr);
  cells[index(i, j, k)].addObject(obj);
}

int Grid::getObjectCount(int i, int j, int k) const {
  return cells[index(i, j, k)].getNumObjects();
}

bool Grid::isOccupied(int i, int j, int k) const {
  return getObjectCount(i, j, k) > 0;
}

void Grid::getWorldBBox(BoundingBox *bb, Matrix *m,
                        Vec3f &wmin, Vec3f &wmax) const {
  Vec3f cmin = bb->getMin();
  Vec3f cmax = bb->getMax();
  if (m == nullptr) {
    wmin = cmin;
    wmax = cmax;
    return;
  }

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

  m->Transform(corners[0]);
  wmin = corners[0];
  wmax = corners[0];
  for (int i = 1; i < 8; i++) {
    m->Transform(corners[i]);
    wmin = Vec3f(::fminf(wmin.x(), corners[i].x()),
                 ::fminf(wmin.y(), corners[i].y()),
                 ::fminf(wmin.z(), corners[i].z()));
    wmax = Vec3f(::fmaxf(wmax.x(), corners[i].x()),
                 ::fmaxf(wmax.y(), corners[i].y()),
                 ::fmaxf(wmax.z(), corners[i].z()));
  }
}

void Grid::voxelIndexRange(Vec3f const&wmin, Vec3f const&wmax,
                           int &i0, int &i1, int &j0, int &j1,
                           int &k0, int &k1) const {
  Vec3f bbMin = sceneBounds->getMin();

  i0 = static_cast<int>((wmin.x() - bbMin.x()) / dx);
  i1 = static_cast<int>((wmax.x() - bbMin.x()) / dx);
  j0 = static_cast<int>((wmin.y() - bbMin.y()) / dy);
  j1 = static_cast<int>((wmax.y() - bbMin.y()) / dy);
  k0 = static_cast<int>((wmin.z() - bbMin.z()) / dz);
  k1 = static_cast<int>((wmax.z() - bbMin.z()) / dz);

  if (i0 < 0) i0 = 0;
  if (j0 < 0) j0 = 0;
  if (k0 < 0) k0 = 0;
  if (i1 >= nx) i1 = nx - 1;
  if (j1 >= ny) j1 = ny - 1;
  if (k1 >= nz) k1 = nz - 1;
}

void Grid::insertObjectInBBox(BoundingBox *bb, Object3D *obj, Matrix *m) {
  if (bb == nullptr || obj == nullptr)
    return;

  Vec3f wmin, wmax;
  getWorldBBox(bb, m, wmin, wmax);
  insertObjectInWorldAABB(wmin, wmax, obj, m);
}

void Grid::insertObjectInWorldAABB(Vec3f const&wminIn, Vec3f const&wmaxIn,
                                   Object3D *obj, Matrix *m) {
  if (obj == nullptr)
    return;

  const float pad = 1e-4f * (dx + dy + dz);
  Vec3f wmin(wminIn.x() - pad, wminIn.y() - pad, wminIn.z() - pad);
  Vec3f wmax(wmaxIn.x() + pad, wmaxIn.y() + pad, wmaxIn.z() + pad);

  int i0, i1, j0, j1, k0, k1;
  voxelIndexRange(wmin, wmax, i0, i1, j0, j1, k0, k1);

  Object3D *stored = wrapForGrid(obj, m);
  for (int i = i0; i <= i1; i++) {
    for (int j = j0; j <= j1; j++) {
      for (int k = k0; k <= k1; k++) {
        Vec3f vmin, vmax;
        getVoxelBounds(i, j, k, vmin, vmax);
        if (boxesOverlap(vmin, vmax, wmin, wmax))
          insertObject(i, j, k, stored);
      }
    }
  }
}

bool Grid::intersectRayBox(Ray const&r, float tmin, float &tEnter, float &tExit,
                           Vec3f &entryNormal) const {

  Vec3f o = r.getOrigin();
  Vec3f d = r.getDirection();
  Vec3f bmin = sceneBounds->getMin();
  Vec3f bmax = sceneBounds->getMax();

  float t0 = 0.0f;
  float t1 = GRID_INF;
  entryNormal = Vec3f(0, 0, 0);

  if (::fabs(d.x()) < GRID_EPSILON) {
    if (o.x() < bmin.x() || o.x() > bmax.x())
      return false;
  } else {
    float tx1 = (bmin.x() - o.x()) / d.x();
    float tx2 = (bmax.x() - o.x()) / d.x();
    Vec3f nx1(-1, 0, 0), nx2(1, 0, 0);
    if (tx1 > tx2) {
      float tmp = tx1; tx1 = tx2; tx2 = tmp;
      Vec3f tn = nx1; nx1 = nx2; nx2 = tn;
    }
    if (tx1 > t0) { t0 = tx1; entryNormal = nx1; }
    if (tx2 < t1) t1 = tx2;
    if (t0 > t1) return false;
  }

  if (::fabs(d.y()) < GRID_EPSILON) {
    if (o.y() < bmin.y() || o.y() > bmax.y())
      return false;
  } else {
    float ty1 = (bmin.y() - o.y()) / d.y();
    float ty2 = (bmax.y() - o.y()) / d.y();
    Vec3f ny1(0, -1, 0), ny2(0, 1, 0);
    if (ty1 > ty2) {
      float tmp = ty1; ty1 = ty2; ty2 = tmp;
      Vec3f tn = ny1; ny1 = ny2; ny2 = tn;
    }
    if (ty1 > t0) { t0 = ty1; entryNormal = ny1; }
    if (ty2 < t1) t1 = ty2;
    if (t0 > t1) return false;
  }

  if (::fabs(d.z()) < GRID_EPSILON) {
    if (o.z() < bmin.z() || o.z() > bmax.z())
      return false;
  } else {
    float tz1 = (bmin.z() - o.z()) / d.z();
    float tz2 = (bmax.z() - o.z()) / d.z();
    Vec3f nz1(0, 0, -1), nz2(0, 0, 1);
    if (tz1 > tz2) {
      float tmp = tz1; tz1 = tz2; tz2 = tmp;
      Vec3f tn = nz1; nz1 = nz2; nz2 = tn;
    }
    if (tz1 > t0) { t0 = tz1; entryNormal = nz1; }
    if (tz2 < t1) t1 = tz2;
    if (t0 > t1) return false;
  }

  tEnter = (t0 > tmin) ? t0 : tmin;
  tExit = t1;
  return tEnter <= tExit;
}

static bool pointInsideBox(Vec3f const&p, BoundingBox const*bb) {
  Vec3f bmin = bb->getMin();
  Vec3f bmax = bb->getMax();
  return p.x() >= bmin.x() && p.x() <= bmax.x() &&
         p.y() >= bmin.y() && p.y() <= bmax.y() &&
         p.z() >= bmin.z() && p.z() <= bmax.z();
}

static void computeVoxelIndex(Vec3f const&p, BoundingBox const*bb,
                              float dx, float dy, float dz,
                              int nx, int ny, int nz,
                              int &i, int &j, int &k) {
  Vec3f bmin = bb->getMin();
  i = static_cast<int>((p.x() - bmin.x()) / dx);
  j = static_cast<int>((p.y() - bmin.y()) / dy);
  k = static_cast<int>((p.z() - bmin.z()) / dz);
  if (i < 0) i = 0;
  if (j < 0) j = 0;
  if (k < 0) k = 0;
  if (i >= nx) i = nx - 1;
  if (j >= ny) j = ny - 1;
  if (k >= nz) k = nz - 1;
}

static void initAxis(float dirComp, float originComp, float bbMinComp,
                     float cellSize, int cellIndex, float tStart,
                     int &sign, float &dT, float &tNext) {

  if (::fabs(dirComp) < GRID_EPSILON) {
    sign = 0;
    dT = GRID_INF;
    tNext = GRID_INF;
    return;
  }

  if (dirComp > 0.0f) {
    sign = 1;
    dT = cellSize / dirComp;
    float nextBoundary = bbMinComp + (cellIndex + 1) * cellSize;
    tNext = (nextBoundary - originComp) / dirComp;
  } else {
    sign = -1;
    dT = -cellSize / dirComp;
    float nextBoundary = bbMinComp + cellIndex * cellSize;
    tNext = (nextBoundary - originComp) / dirComp;
  }

  while (tNext < tStart)
    tNext += dT;
}

void Grid::initializeRayMarch(MarchingInfo &mi, Ray const&r, float tmin) const {
  mi.setValid(false);

  Vec3f origin = r.getOrigin();
  Vec3f dir = r.getDirection();
  Vec3f bbMin = sceneBounds->getMin();

  float tEnterBox, tExitBox;
  Vec3f entryNormal;
  bool hitsBox = intersectRayBox(r, tmin, tEnterBox, tExitBox, entryNormal);
  bool inside = pointInsideBox(origin, sceneBounds);
  float tEnter, tExit;

  if (inside) {

    tEnter = tmin;
    if (!hitsBox)
      return;
    tExit = tExitBox;
    entryNormal = Vec3f(0, 0, 0);
  } else {

    if (!hitsBox || tEnterBox > tExitBox)
      return;
    tEnter = tEnterBox;
    tExit = tExitBox;
  }

  Vec3f startPoint = r.pointAtParameter(tEnter);
  int i, j, k;
  computeVoxelIndex(startPoint, sceneBounds, dx, dy, dz, nx, ny, nz, i, j, k);

  int signX, signY, signZ;
  float dTx, dTy, dTz, tNextX, tNextY, tNextZ;
  initAxis(dir.x(), origin.x(), bbMin.x(), dx, i, tEnter, signX, dTx, tNextX);
  initAxis(dir.y(), origin.y(), bbMin.y(), dy, j, tEnter, signY, dTy, tNextY);
  initAxis(dir.z(), origin.z(), bbMin.z(), dz, k, tEnter, signZ, dTz, tNextZ);

  mi.setTMin(tEnter);
  mi.setTExit(tExit);
  mi.setI(i);
  mi.setJ(j);
  mi.setK(k);
  mi.setSignX(signX);
  mi.setSignY(signY);
  mi.setSignZ(signZ);
  mi.setDTx(dTx);
  mi.setDTy(dTy);
  mi.setDTz(dTz);
  mi.setTNextX(tNextX);
  mi.setTNextY(tNextY);
  mi.setTNextZ(tNextZ);
  mi.setNormal(entryNormal);
  mi.setValid(true);
}

bool Grid::intersect(Ray const&r, Hit &h, float tmin) {
  return false;
}

bool Grid::intersectShadow(Ray const&r, float tmin, float tmax, float &t,
                           Material **outMaterial) {
  return false;
}
