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

// 网格内存储的变换包装器
class GridTransform : public Object3D {

public:
  GridTransform(const Matrix &m, Object3D *o) : matrix(m), object(o) {
    matrix.Inverse(inverseMatrix);
    inverseMatrix.Transpose();
  }

  // 射线-物体求交
  virtual bool intersect(const Ray &r, Hit &h, float tmin) {
    Matrix objectMatrix;
    matrix.Inverse(objectMatrix);

    // 变换射线
    Vec3f origin = r.getOrigin();
    Vec3f direction = r.getDirection();
    objectMatrix.Transform(origin);
    objectMatrix.TransformDirection(direction);

    // 局部射线
    Ray localRay(origin, direction);
    Hit localHit(h);
    if (!object->intersect(localRay, localHit, tmin))  // 局部求交
      return false;

    // 变换法线
    Vec3f normal = localHit.getNormal();
    inverseMatrix.TransformDirection(normal);
    normal.Normalize();
    h.set(localHit.getT(), localHit.getMaterial(), normal, r);  // 设置交点
    return true;
  }

  // 射线-物体求交阴影
  virtual bool intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                               Material **outMaterial) {
    Matrix objectMatrix;
    matrix.Inverse(objectMatrix);

    Vec3f origin = r.getOrigin();
    Vec3f direction = r.getDirection();
    objectMatrix.Transform(origin);
    objectMatrix.TransformDirection(direction);

    Ray localRay(origin, direction);  // 局部射线
    return object->intersectShadow(localRay, tmin, tmax, t, outMaterial);  // 局部求交阴影
  }

private:
  Matrix matrix;
  Matrix inverseMatrix;
  Object3D *object;
};

// 判断两个包围盒是否重叠
static bool boxesOverlap(const Vec3f &amin, const Vec3f &amax,
                         const Vec3f &bmin, const Vec3f &bmax) {
  return amin.x() <= bmax.x() && amax.x() >= bmin.x() &&
         amin.y() <= bmax.y() && amax.y() >= bmin.y() &&
         amin.z() <= bmax.z() && amax.z() >= bmin.z();
}

// 构造体素网格
Grid::Grid(BoundingBox *bb, int nx, int ny, int nz)
    : sceneBounds(NULL), nx(nx), ny(ny), nz(nz),
      dx(0), dy(0), dz(0), cells(NULL),
      infiniteObjects(NULL), gridWrappers(NULL) {
  // 边界检查
  assert(bb != NULL);
  assert(nx > 0 && ny > 0 && nz > 0);

  // 创建场景包围盒
  Vec3f bbMin = bb->getMin();
  Vec3f bbMax = bb->getMax();
  sceneBounds = new BoundingBox(bbMin, bbMax);

  // 计算体素网格边长
  dx = (bbMax.x() - bbMin.x()) / nx;
  dy = (bbMax.y() - bbMin.y()) / ny;
  dz = (bbMax.z() - bbMin.z()) / nz;

  // 创建体素网格
  int numCells = nx * ny * nz;
  cells = new Object3DVector[numCells];
  infiniteObjects = new Object3DVector();
  gridWrappers = new Object3DVector();

  material = NULL;
}

Grid::~Grid() {
  if (gridWrappers != NULL) {
    for (int i = 0; i < gridWrappers->getNumObjects(); i++)
      delete gridWrappers->getObject(i);
    delete gridWrappers;
  }
  gridWrappers = NULL;

  delete infiniteObjects;
  infiniteObjects = NULL;

  material = NULL;

  delete [] cells;
  cells = NULL;
  delete sceneBounds;
  sceneBounds = NULL;
}

// 获取体素索引
int Grid::index(int i, int j, int k) const {
  assert(i >= 0 && i < nx);
  assert(j >= 0 && j < ny);
  assert(k >= 0 && k < nz);
  return i * ny * nz + j * nz + k;
}

// 判断体素是否在边界内
bool Grid::inBounds(int i, int j, int k) const {
  return i >= 0 && i < nx && j >= 0 && j < ny && k >= 0 && k < nz;
}

// 获取体素包围盒
void Grid::getVoxelBounds(int i, int j, int k, Vec3f &vmin, Vec3f &vmax) const {
  Vec3f bbMin = sceneBounds->getMin();
  vmin = Vec3f(bbMin.x() + i * dx,
               bbMin.y() + j * dy,
               bbMin.z() + k * dz);
  vmax = Vec3f(vmin.x() + dx, vmin.y() + dy, vmin.z() + dz);
}

// 获取体素中心点
Vec3f Grid::getVoxelCenter(int i, int j, int k) const {
  Vec3f vmin, vmax;
  getVoxelBounds(i, j, k, vmin, vmax);
  return (vmin + vmax) * 0.5f;
}

// 获取体素半对角线长度
float Grid::getVoxelHalfDiagonal() const {
  return 0.5f * sqrtf(dx * dx + dy * dy + dz * dz);
}

// 将带累积变换的图元包装后存入网格
Object3D *Grid::wrapForGrid(Object3D *obj, Matrix *m) {
  if (m == NULL)
    return obj;
  GridTransform *wrapper = new GridTransform(*m, obj);
  gridWrappers->addObject(wrapper);
  return wrapper;
}

// 添加无限图元
void Grid::addInfiniteObject(Object3D *obj) {
  assert(obj != NULL);
  infiniteObjects->addObject(obj);
}

// 获取体素
Object3DVector *Grid::getCell(int i, int j, int k) {
  return &cells[index(i, j, k)];
}

// 插入物体到体素网格
void Grid::insertObject(int i, int j, int k, Object3D *obj) {
  assert(obj != NULL);
  cells[index(i, j, k)].addObject(obj);
}

// 获取体素内物体数量
int Grid::getObjectCount(int i, int j, int k) const {
  return cells[index(i, j, k)].getNumObjects();
}

// 判断体素是否被占用
bool Grid::isOccupied(int i, int j, int k) const {
  return getObjectCount(i, j, k) > 0;
}

// 获取物体包围盒
void Grid::getWorldBBox(BoundingBox *bb, Matrix *m,
                        Vec3f &wmin, Vec3f &wmax) const {
  Vec3f cmin = bb->getMin();
  Vec3f cmax = bb->getMax();
  if (m == NULL) {
    wmin = cmin;
    wmax = cmax;
    return;
  }

  // 获取包围盒的8个顶点
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

  // 变换包围盒顶点
  m->Transform(corners[0]);
  wmin = corners[0];
  wmax = corners[0];
  for (int i = 1; i < 8; i++) {
    m->Transform(corners[i]);
    wmin = Vec3f(fminf(wmin.x(), corners[i].x()),
                 fminf(wmin.y(), corners[i].y()),
                 fminf(wmin.z(), corners[i].z()));
    wmax = Vec3f(fmaxf(wmax.x(), corners[i].x()),
                 fmaxf(wmax.y(), corners[i].y()),
                 fmaxf(wmax.z(), corners[i].z()));
  }
}

// 获取体素索引范围
void Grid::voxelIndexRange(const Vec3f &wmin, const Vec3f &wmax,
                           int &i0, int &i1, int &j0, int &j1,
                           int &k0, int &k1) const {
  Vec3f bbMin = sceneBounds->getMin();

  // 计算体素索引范围
  i0 = (int)((wmin.x() - bbMin.x()) / dx);
  i1 = (int)((wmax.x() - bbMin.x()) / dx);
  j0 = (int)((wmin.y() - bbMin.y()) / dy);
  j1 = (int)((wmax.y() - bbMin.y()) / dy);
  k0 = (int)((wmin.z() - bbMin.z()) / dz);
  k1 = (int)((wmax.z() - bbMin.z()) / dz);

  // 边界修正
  if (i0 < 0) i0 = 0;
  if (j0 < 0) j0 = 0;
  if (k0 < 0) k0 = 0;
  if (i1 >= nx) i1 = nx - 1;
  if (j1 >= ny) j1 = ny - 1;
  if (k1 >= nz) k1 = nz - 1;
}

// 插入物体到包围盒
void Grid::insertObjectInBBox(BoundingBox *bb, Object3D *obj, Matrix *m) {
  if (bb == NULL || obj == NULL)
    return;

  // 获取包围盒的世界坐标
  Vec3f wmin, wmax;
  getWorldBBox(bb, m, wmin, wmax);
  insertObjectInWorldAABB(wmin, wmax, obj, m);
}

// 用世界空间 AABB 插入体素
void Grid::insertObjectInWorldAABB(const Vec3f &wminIn, const Vec3f &wmaxIn,
                                   Object3D *obj, Matrix *m) {
  if (obj == NULL)
    return;

  const float pad = 1e-4f * (dx + dy + dz);
  Vec3f wmin(wminIn.x() - pad, wminIn.y() - pad, wminIn.z() - pad);
  Vec3f wmax(wmaxIn.x() + pad, wmaxIn.y() + pad, wmaxIn.z() + pad);

  // 获取体素索引范围
  int i0, i1, j0, j1, k0, k1;
  voxelIndexRange(wmin, wmax, i0, i1, j0, j1, k0, k1);

  // 插入物体到包围盒内的体素
  Object3D *stored = wrapForGrid(obj, m);
  for (int i = i0; i <= i1; i++) {
    for (int j = j0; j <= j1; j++) {
      for (int k = k0; k <= k1; k++) {
        Vec3f vmin, vmax;
        getVoxelBounds(i, j, k, vmin, vmax);
        if (boxesOverlap(vmin, vmax, wmin, wmax))  // 体素包围盒与包围盒重叠
          insertObject(i, j, k, stored);
      }
    }
  }
}

// 射线-包围盒求交
bool Grid::intersectRayBox(const Ray &r, float tmin, float &tEnter, float &tExit,
                           Vec3f &entryNormal) const {
  // 获取射线起点和方向
  Vec3f o = r.getOrigin();
  Vec3f d = r.getDirection();
  Vec3f bmin = sceneBounds->getMin();
  Vec3f bmax = sceneBounds->getMax();

  float t0 = 0.0f; // 进入参数
  float t1 = GRID_INF; // 离开参数
  entryNormal = Vec3f(0, 0, 0); // 进入面法线

  // 调整射线起点和方向
  // x 轴
  if (fabs(d.x()) < GRID_EPSILON) {
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

  // y 轴
  if (fabs(d.y()) < GRID_EPSILON) {
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

  // z 轴
  if (fabs(d.z()) < GRID_EPSILON) {
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

  // 更新进入参数和离开参数
  tEnter = (t0 > tmin) ? t0 : tmin;
  tExit = t1;
  return tEnter <= tExit;
}

// 判断点是否在场景包围盒内部
static bool pointInsideBox(const Vec3f &p, const BoundingBox *bb) {
  Vec3f bmin = bb->getMin();
  Vec3f bmax = bb->getMax();
  return p.x() >= bmin.x() && p.x() <= bmax.x() &&
         p.y() >= bmin.y() && p.y() <= bmax.y() &&
         p.z() >= bmin.z() && p.z() <= bmax.z();
}

// 由空间点计算体素索引
static void computeVoxelIndex(const Vec3f &p, const BoundingBox *bb,
                              float dx, float dy, float dz,
                              int nx, int ny, int nz,
                              int &i, int &j, int &k) {
  Vec3f bmin = bb->getMin();
  i = (int)((p.x() - bmin.x()) / dx);
  j = (int)((p.y() - bmin.y()) / dy);
  k = (int)((p.z() - bmin.z()) / dz);
  if (i < 0) i = 0;
  if (j < 0) j = 0;
  if (k < 0) k = 0;
  if (i >= nx) i = nx - 1;
  if (j >= ny) j = ny - 1;
  if (k >= nz) k = nz - 1;
}

// 初始化单轴 DDA 参数
static void initAxis(float dirComp, float originComp, float bbMinComp,
                     float cellSize, int cellIndex, float tStart,
                     int &sign, float &dT, float &tNext) {
  // 射线方向不为 0
  if (fabs(dirComp) < GRID_EPSILON) {
    sign = 0;
    dT = GRID_INF;
    tNext = GRID_INF;
    return;
  }

  // 射线方向为正
  if (dirComp > 0.0f) {
    sign = 1;
    dT = cellSize / dirComp;
    float nextBoundary = bbMinComp + (cellIndex + 1) * cellSize;
    tNext = (nextBoundary - originComp) / dirComp;
  } else {  // 射线方向为负
    sign = -1;
    dT = -cellSize / dirComp;
    float nextBoundary = bbMinComp + cellIndex * cellSize;
    tNext = (nextBoundary - originComp) / dirComp;
  }

  // 更新步进参数
  while (tNext < tStart)
    tNext += dT;
}

// 初始化步进状态（3D DDA）
// 包含三种情况：原点在网格外命中、原点在网格外射失、原点在网格内
void Grid::initializeRayMarch(MarchingInfo &mi, const Ray &r, float tmin) const {
  mi.setValid(false);

  Vec3f origin = r.getOrigin();  // 射线起点
  Vec3f dir = r.getDirection();  // 射线方向
  Vec3f bbMin = sceneBounds->getMin();  // 包围盒最小点

  float tEnterBox, tExitBox;  // 包围盒交点
  Vec3f entryNormal;  // 进入面法线
  bool hitsBox = intersectRayBox(r, tmin, tEnterBox, tExitBox, entryNormal);  // 射线是否与包围盒相交
  bool inside = pointInsideBox(origin, sceneBounds);  // 射线起点是否在包围盒内
  float tEnter, tExit;  // 进入参数和离开参数

  if (inside) {
    // 原点在网格外壳内部-从 tmin 开始步进，以包围盒出口为终止
    tEnter = tmin;
    if (!hitsBox)
      return;
    tExit = tExitBox;
    entryNormal = Vec3f(0, 0, 0);
  } else {
    // 原点在网格外壳外部-从包围盒入口开始步进，以包围盒出口为终止
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

// 取消可视化使用，优化编译，加速求交由 RayTracer 通过 DDA 完成
bool Grid::intersect(const Ray &r, Hit &h, float tmin) {
  return false;
}

// 求阴影（3D DDA）
bool Grid::intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                           Material **outMaterial) {
  return false;  // 未命中
}
