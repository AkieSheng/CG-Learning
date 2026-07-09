#include "gl_headers.h"
#include "grid.h"
#include "boundingbox.h"
#include "marchinginfo.h"
#include "material.h"
#include "ray.h"
#include "hit.h"
#include "rayTree.h"
#include "matrix.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static const float GRID_EPSILON = 1.0e-6f;
static const float GRID_INF = 1.0e30f;
static const int GRADIENT_LEVELS = 7;

// [DEBUG] 白-绿-青-蓝-黄-橙-红（可视化）
static const float GRADIENT_COLORS[GRADIENT_LEVELS][3] = {
  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f},
  {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.5f, 0.0f},
  {1.0f, 0.0f, 0.0f}
};

static int gradientIndex(int value) {
  if (value < 0) return 0;
  if (value >= GRADIENT_LEVELS) return GRADIENT_LEVELS - 1;
  return value;
}

static Vec3f getGradientColor(int index) {
  int idx = gradientIndex(index);
  return Vec3f(GRADIENT_COLORS[idx][0], GRADIENT_COLORS[idx][1],
               GRADIENT_COLORS[idx][2]);
}

// [DEBUG] Ray Tree 调试（按 DDA 遍历顺序着色）
static Material *getTraversalMaterial(int step) {
  static PhongMaterial *materials[GRADIENT_LEVELS] = {NULL};
  if (materials[0] == NULL) {
    for (int i = 0; i < GRADIENT_LEVELS; i++) {
      Vec3f color = getGradientColor(i);
      materials[i] = new PhongMaterial(
          color, Vec3f(0.1f, 0.1f, 0.1f), 10.0f,
          Vec3f(0, 0, 0), Vec3f(0, 0, 0), 1.0f);
    }
  }
  return materials[gradientIndex(step)];
}

// 判断两个包围盒是否重叠
static bool boxesOverlap(const Vec3f &amin, const Vec3f &amax,
                         const Vec3f &bmin, const Vec3f &bmax) {
  return amin.x() <= bmax.x() && amax.x() >= bmin.x() &&
         amin.y() <= bmax.y() && amax.y() >= bmin.y() &&
         amin.z() <= bmax.z() && amax.z() >= bmin.z();
}

// [DEBUG] 将体素密度映射为字符
static char densityChar(int count) {
  if (count <= 0) return '.';
  if (count == 1) return '#';
  if (count < 10) return (char)('0' + count);
  return '*';
}

// 构造体素网格
Grid::Grid(BoundingBox *bb, int nx, int ny, int nz)
    : sceneBounds(NULL), nx(nx), ny(ny), nz(nz),
      dx(0), dy(0), dz(0), cells(NULL), densityMaterials(NULL) {
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

  // 创建密度着色材质
  densityMaterials = new PhongMaterial*[MAX_DENSITY_LEVELS];
  for (int i = 0; i < MAX_DENSITY_LEVELS; i++) {
    Vec3f color = getDensityColor(i + 1);
    densityMaterials[i] = new PhongMaterial(
        color, Vec3f(0.1f, 0.1f, 0.1f), 10.0f,
        Vec3f(0, 0, 0), Vec3f(0, 0, 0), 1.0f);
  }
  material = densityMaterials[0];
}

Grid::~Grid() {
  if (densityMaterials != NULL) {
    for (int i = 0; i < MAX_DENSITY_LEVELS; i++)
      delete densityMaterials[i];
    delete [] densityMaterials;
  }
  densityMaterials = NULL;
  material = NULL;

  delete [] cells;
  cells = NULL;
  delete sceneBounds;
  sceneBounds = NULL;
}

// 栅格密度着色：按重叠图元数量着色
Vec3f Grid::getDensityColor(int count) const {
  if (count <= 0)
    return Vec3f(0, 0, 0);
  return getGradientColor(count - 1);
}

// 获取密度着色材质
Material *Grid::getDensityMaterial(int count) const {
  if (count <= 0)
    return material;
  int idx = count - 1;
  if (idx >= MAX_DENSITY_LEVELS)
    idx = MAX_DENSITY_LEVELS - 1;
  return densityMaterials[idx];
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
  Vec3f bbMax = sceneBounds->getMax();
  vmin = Vec3f(bbMin.x() + i * dx,
               bbMin.y() + j * dy,
               bbMin.z() + k * dz);
  vmax = Vec3f(vmin.x() + dx, vmin.y() + dy, vmin.z() + dz);
  // 边界体素对齐场景 AABB，避免浮点缝隙
  if (i == 0) vmin = Vec3f(bbMin.x(), vmin.y(), vmin.z());
  if (j == 0) vmin = Vec3f(vmin.x(), bbMin.y(), vmin.z());
  if (k == 0) vmin = Vec3f(vmin.x(), vmin.y(), bbMin.z());
  if (i == nx - 1) vmax = Vec3f(bbMax.x(), vmax.y(), vmax.z());
  if (j == ny - 1) vmax = Vec3f(vmax.x(), bbMax.y(), vmax.z());
  if (k == nz - 1) vmax = Vec3f(vmax.x(), vmax.y(), bbMax.z());
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
  if (i0 >= nx) i0 = nx - 1;
  if (j0 >= ny) j0 = ny - 1;
  if (k0 >= nz) k0 = nz - 1;
  if (i1 >= nx) i1 = nx - 1;
  if (j1 >= ny) j1 = ny - 1;
  if (k1 >= nz) k1 = nz - 1;
  if (i0 > i1) i0 = i1;
  if (j0 > j1) j0 = j1;
  if (k0 > k1) k0 = k1;
}

// 插入物体到包围盒
void Grid::insertObjectInBBox(BoundingBox *bb, Object3D *obj, Matrix *m) {
  if (bb == NULL || obj == NULL)
    return;

  // 获取包围盒的世界坐标
  Vec3f wmin, wmax;
  getWorldBBox(bb, m, wmin, wmax);

  // 获取体素索引范围
  int i0, i1, j0, j1, k0, k1;
  voxelIndexRange(wmin, wmax, i0, i1, j0, j1, k0, k1);

  // 插入物体到包围盒内的体素
  for (int i = i0; i <= i1; i++) {
    for (int j = j0; j <= j1; j++) {
      for (int k = k0; k <= k1; k++) {
        // 获取体素包围盒
        Vec3f vmin, vmax;
        getVoxelBounds(i, j, k, vmin, vmax);
        // 边界检查
        if (boxesOverlap(vmin, vmax, wmin, wmax))
          insertObject(i, j, k, obj);
      }
    }
  }
}

// 获取被占用的体素数量
int Grid::getOccupiedCount() const {
  int count = 0;
  int numCells = nx * ny * nz;
  for (int idx = 0; idx < numCells; idx++) {
    if (cells[idx].getNumObjects() > 0)
      count++;
  }
  return count;
}

// [DEBUG] 打印占用情况
void Grid::printOccupancy() const {
  printf("Grid occupancy (%d x %d x %d), non-empty cells: %d / %d\n",
         nx, ny, nz, getOccupiedCount(), nx * ny * nz);
  for (int k = 0; k < nz; k++) {
    printf("--- slice k = %d ---\n", k);
    for (int j = ny - 1; j >= 0; j--) {
      for (int i = 0; i < nx; i++)
        printf("%c", densityChar(getObjectCount(i, j, k)));
      printf("\n");
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

// 添加射线命中体素（按遍历顺序着色）
void Grid::addRayTreeHitCell(int i, int j, int k, int step) const {
  Material *cellMat = getTraversalMaterial(step);
  Vec3f vmin, vmax;
  getVoxelBounds(i, j, k, vmin, vmax);  // 体素包围盒

  // 体素8个顶点
  Vec3f p000(vmin.x(), vmin.y(), vmin.z());
  Vec3f p100(vmax.x(), vmin.y(), vmin.z());
  Vec3f p110(vmax.x(), vmax.y(), vmin.z());
  Vec3f p010(vmin.x(), vmax.y(), vmin.z());
  Vec3f p001(vmin.x(), vmin.y(), vmax.z());
  Vec3f p101(vmax.x(), vmin.y(), vmax.z());
  Vec3f p111(vmax.x(), vmax.y(), vmax.z());
  Vec3f p011(vmin.x(), vmax.y(), vmax.z());

  // 射线命中体素面
  RayTree::AddHitCellFace(p000, p100, p110, p010, Vec3f(0, 0, -1), cellMat);
  RayTree::AddHitCellFace(p001, p101, p111, p011, Vec3f(0, 0, 1), cellMat);
  RayTree::AddHitCellFace(p000, p010, p011, p001, Vec3f(-1, 0, 0), cellMat);
  RayTree::AddHitCellFace(p100, p110, p111, p101, Vec3f(1, 0, 0), cellMat);
  RayTree::AddHitCellFace(p000, p100, p101, p001, Vec3f(0, -1, 0), cellMat);
  RayTree::AddHitCellFace(p010, p110, p111, p011, Vec3f(0, 1, 0), cellMat);
}

// 添加射线遍历体素
void Grid::addRayTreeTraversal(int i, int j, int k, const Vec3f &entryNormal,
                               int step) const {
  addRayTreeHitCell(i, j, k, step);

  Material *cellMat = getTraversalMaterial(step);
  Vec3f vmin, vmax;
  getVoxelBounds(i, j, k, vmin, vmax);
  Vec3f n = entryNormal;

  // 根据进入面法线选择对应四边形，可视化进入面
  if (n.x() < -0.5f) {
    RayTree::AddEnteredFace(
        Vec3f(vmin.x(), vmin.y(), vmin.z()),
        Vec3f(vmin.x(), vmax.y(), vmin.z()),
        Vec3f(vmin.x(), vmax.y(), vmax.z()),
        Vec3f(vmin.x(), vmin.y(), vmax.z()), n, cellMat);
  } else if (n.x() > 0.5f) {
    RayTree::AddEnteredFace(
        Vec3f(vmax.x(), vmin.y(), vmin.z()),
        Vec3f(vmax.x(), vmin.y(), vmax.z()),
        Vec3f(vmax.x(), vmax.y(), vmax.z()),
        Vec3f(vmax.x(), vmax.y(), vmin.z()), n, cellMat);
  } else if (n.y() < -0.5f) {
    RayTree::AddEnteredFace(
        Vec3f(vmin.x(), vmin.y(), vmin.z()),
        Vec3f(vmax.x(), vmin.y(), vmin.z()),
        Vec3f(vmax.x(), vmin.y(), vmax.z()),
        Vec3f(vmin.x(), vmin.y(), vmax.z()), n, cellMat);
  } else if (n.y() > 0.5f) {
    RayTree::AddEnteredFace(
        Vec3f(vmin.x(), vmax.y(), vmin.z()),
        Vec3f(vmin.x(), vmax.y(), vmax.z()),
        Vec3f(vmax.x(), vmax.y(), vmax.z()),
        Vec3f(vmax.x(), vmax.y(), vmin.z()), n, cellMat);
  } else if (n.z() < -0.5f) {
    RayTree::AddEnteredFace(
        Vec3f(vmin.x(), vmin.y(), vmin.z()),
        Vec3f(vmax.x(), vmin.y(), vmin.z()),
        Vec3f(vmax.x(), vmax.y(), vmin.z()),
        Vec3f(vmin.x(), vmax.y(), vmin.z()), n, cellMat);
  } else if (n.z() > 0.5f) {
    RayTree::AddEnteredFace(
        Vec3f(vmin.x(), vmin.y(), vmax.z()),
        Vec3f(vmin.x(), vmax.y(), vmax.z()),
        Vec3f(vmax.x(), vmax.y(), vmax.z()),
        Vec3f(vmax.x(), vmin.y(), vmax.z()), n, cellMat);
  }
}

// 求交（3D DDA）
bool Grid::intersect(const Ray &r, Hit &h, float tmin) {
  MarchingInfo mi;  // 步进信息
  initializeRayMarch(mi, r, tmin);
  if (!mi.isValid())
    return false;

  // 沿射线步进，命中第一个占用体素
  int step = 0;
  while (mi.getTMin() <= mi.getTExit()) {
    int i = mi.getI();
    int j = mi.getJ();
    int k = mi.getK();

    if (!inBounds(i, j, k))
      break;

    addRayTreeTraversal(i, j, k, mi.getNormal(), step);

    int count = getObjectCount(i, j, k);
    if (count > 0 && mi.getTMin() >= tmin && mi.getTMin() < h.getT()) {
      h.set(mi.getTMin(), getDensityMaterial(count), mi.getNormal(), r);
      return true;  // 体素被占用且射线起点在体素内，命中
    }

    mi.nextCell();
    step++;
  }
  return false;  // 未命中
}

// 求阴影（3D DDA）
bool Grid::intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                           Material **outMaterial) {
  return false;  // 未命中
}

// 按密度着色，绘制外露面
void Grid::paintVoxelFace(const Vec3f &a, const Vec3f &b, const Vec3f &c,
                          const Vec3f &d, const Vec3f &normal, int count) const {
  Material *cellMat = getDensityMaterial(count);  // 体素材质
  if (cellMat != NULL)
    cellMat->glSetMaterial();
  glNormal3f(normal.x(), normal.y(), normal.z());  // 法线
  glBegin(GL_QUADS);  // 绘制四边形
  glVertex3f(a.x(), a.y(), a.z());
  glVertex3f(b.x(), b.y(), b.z());
  glVertex3f(c.x(), c.y(), c.z());
  glVertex3f(d.x(), d.y(), d.z());
  glEnd();
}

// 绘制体素
void Grid::paint(void) const {
  glEnable(GL_LIGHTING);  // 启用光照

  // 遍历体素
  for (int i = 0; i < nx; i++) {
    for (int j = 0; j < ny; j++) {
      for (int k = 0; k < nz; k++) {
        int count = getObjectCount(i, j, k);
        if (count <= 0)
          continue;

        // 获取体素包围盒
        Vec3f vmin, vmax;
        getVoxelBounds(i, j, k, vmin, vmax);

        // 绘制体素面
        if (i == 0 || getObjectCount(i - 1, j, k) == 0)  // 左面
          paintVoxelFace(
              Vec3f(vmin.x(), vmin.y(), vmin.z()),
              Vec3f(vmin.x(), vmin.y(), vmax.z()),
              Vec3f(vmin.x(), vmax.y(), vmax.z()),
              Vec3f(vmin.x(), vmax.y(), vmin.z()),
              Vec3f(-1, 0, 0), count);
        if (i == nx - 1 || getObjectCount(i + 1, j, k) == 0)
          paintVoxelFace(  // 右面
              Vec3f(vmax.x(), vmin.y(), vmin.z()),
              Vec3f(vmax.x(), vmax.y(), vmin.z()),
              Vec3f(vmax.x(), vmax.y(), vmax.z()),
              Vec3f(vmax.x(), vmin.y(), vmax.z()),
              Vec3f(1, 0, 0), count);
        if (j == 0 || getObjectCount(i, j - 1, k) == 0)
          paintVoxelFace(  // 前面
              Vec3f(vmin.x(), vmin.y(), vmin.z()),
              Vec3f(vmax.x(), vmin.y(), vmin.z()),
              Vec3f(vmax.x(), vmin.y(), vmax.z()),
              Vec3f(vmin.x(), vmin.y(), vmax.z()),
              Vec3f(0, -1, 0), count);
        if (j == ny - 1 || getObjectCount(i, j + 1, k) == 0)
          paintVoxelFace(  // 后面
              Vec3f(vmin.x(), vmax.y(), vmin.z()),
              Vec3f(vmin.x(), vmax.y(), vmax.z()),
              Vec3f(vmax.x(), vmax.y(), vmax.z()),
              Vec3f(vmax.x(), vmax.y(), vmin.z()),
              Vec3f(0, 1, 0), count);
        if (k == 0 || getObjectCount(i, j, k - 1) == 0)
          paintVoxelFace(  // 下面
              Vec3f(vmin.x(), vmin.y(), vmin.z()),
              Vec3f(vmax.x(), vmin.y(), vmin.z()),
              Vec3f(vmax.x(), vmax.y(), vmin.z()),
              Vec3f(vmin.x(), vmax.y(), vmin.z()),
              Vec3f(0, 0, -1), count);
        if (k == nz - 1 || getObjectCount(i, j, k + 1) == 0)
          paintVoxelFace(  // 上面
              Vec3f(vmin.x(), vmin.y(), vmax.z()),
              Vec3f(vmin.x(), vmax.y(), vmax.z()),
              Vec3f(vmax.x(), vmax.y(), vmax.z()),
              Vec3f(vmax.x(), vmin.y(), vmax.z()),
              Vec3f(0, 0, 1), count);
      }
    }
  }
}
