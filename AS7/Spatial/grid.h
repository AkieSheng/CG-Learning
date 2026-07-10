#ifndef _GRID_H_
#define _GRID_H_

#include "object3d.h"
#include "object3dvector.h"

class BoundingBox;
class MarchingInfo;
class Ray;
class Hit;
class Matrix;

// 均匀体素网格
class Grid : public Object3D {

public:
  Grid(BoundingBox *bb, int nx, int ny, int nz);
  virtual ~Grid();

  BoundingBox *getBoundingBox() const { return sceneBounds; }

  int getNX() const { return nx; }
  int getNY() const { return ny; }
  int getNZ() const { return nz; }

  Vec3f getVoxelCenter(int i, int j, int k) const;  // 获取体素中心点
  float getVoxelHalfDiagonal() const;  // 获取体素半对角线长度

  void insertObject(int i, int j, int k, Object3D *obj);  // 将物体插入体素网格
  int getObjectCount(int i, int j, int k) const;  // 获取体素内物体数量
  bool isOccupied(int i, int j, int k) const;  // 获取体素占用标记

  void insertObjectInBBox(BoundingBox *bb, Object3D *obj, Matrix *m = NULL);  // 将物体插入包围盒
  void insertObjectInWorldAABB(const Vec3f &wmin, const Vec3f &wmax,
                               Object3D *obj, Matrix *m = NULL);

  void initializeRayMarch(MarchingInfo &mi, const Ray &r, float tmin) const;

  void addInfiniteObject(Object3D *obj);  // 添加无限图元
  Object3DVector *getCell(int i, int j, int k);  // 获取体素
  const Object3DVector &getInfiniteObjects() const { return *infiniteObjects; }  // 获取无限图元

  Object3D *wrapForGrid(Object3D *obj, Matrix *m);  // 包装物体

  bool inBounds(int i, int j, int k) const;  // 判断体素是否在边界内

  virtual bool intersect(const Ray &r, Hit &h, float tmin);  // 求交
  virtual bool intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                               Material **outMaterial);  // 求交阴影

private:
  int index(int i, int j, int k) const;  // 获取体素索引
  void getVoxelBounds(int i, int j, int k, Vec3f &vmin, Vec3f &vmax) const;  // 获取体素边界
  void getWorldBBox(BoundingBox *bb, Matrix *m, Vec3f &wmin, Vec3f &wmax) const;  // 获取世界边界
  void voxelIndexRange(const Vec3f &wmin, const Vec3f &wmax,  // 获取体素索引范围
                       int &i0, int &i1, int &j0, int &j1, int &k0, int &k1) const;  // 获取体素索引范围

  // 射线与包围盒求交，返回进入/离开参数
  bool intersectRayBox(const Ray &r, float tmin, float &tEnter, float &tExit,
                       Vec3f &entryNormal) const;

  BoundingBox *sceneBounds;  // 场景包围盒
  int nx, ny, nz;  // 体素网格分辨率
  float dx, dy, dz;  // 体素网格边长
  Object3DVector *cells;  // 体素网格
  Object3DVector *infiniteObjects;  // 无限图元
  Object3DVector *gridWrappers;  // Transform 包装器
};

#endif
