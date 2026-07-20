#pragma once

#include "object3d.h"
#include "object3dvector.h"

struct BoundingBox;
struct MarchingInfo;
struct Ray;
struct Hit;
struct Matrix;
struct PhongMaterial;

struct Grid : Object3D {
  Grid(BoundingBox* bb, int nx, int ny, int nz);
  ~Grid() override;

  auto getBoundingBox() const -> BoundingBox* { return sceneBounds; }

  auto getNX() const -> int { return nx; }
  auto getNY() const -> int { return ny; }
  auto getNZ() const -> int { return nz; }

  auto getVoxelCenter(int i, int j, int k) const -> Vec3f;
  auto getVoxelHalfDiagonal() const -> float;

  auto insertObject(int i, int j, int k, Object3D* obj) -> void;
  auto getObjectCount(int i, int j, int k) const -> int;
  auto isOccupied(int i, int j, int k) const -> bool;

  auto insertObjectInBBox(BoundingBox* bb, Object3D* obj, Matrix* m = nullptr) -> void;
  auto insertObjectInWorldAABB(Vec3f const& wmin, Vec3f const& wmax,
                               Object3D* obj, Matrix* m = nullptr) -> void;

  auto printOccupancy() const -> void;

  auto getDensityColor(int count) const -> Vec3f;
  auto getDensityMaterial(int count) const -> Material*;

  auto initializeRayMarch(MarchingInfo& mi, Ray const& r, float tmin) const -> void;

  auto addInfiniteObject(Object3D* obj) -> void;
  auto getCell(int i, int j, int k) -> Object3DVector*;
  auto getInfiniteObjects() const -> Object3DVector const& {
    return *infiniteObjects;
  }

  auto wrapForGrid(Object3D* obj, Matrix* m) -> Object3D*;

  auto inBounds(int i, int j, int k) const -> bool;

  auto intersect(Ray const& r, Hit& h, float tmin) -> bool override;
  auto intersectShadow(Ray const& r, float tmin, float tmax, float& t,
                       Material** outMaterial) -> bool override;
  auto paint() const -> void override;

private:
  static int const MAX_DENSITY_LEVELS = 16;

  auto index(int i, int j, int k) const -> int;
  auto getVoxelBounds(int i, int j, int k, Vec3f& vmin, Vec3f& vmax) const -> void;
  auto getWorldBBox(BoundingBox* bb, Matrix* m, Vec3f& wmin, Vec3f& wmax) const -> void;
  auto voxelIndexRange(Vec3f const& wmin, Vec3f const& wmax,
                       int& i0, int& i1, int& j0, int& j1, int& k0, int& k1) const -> void;

  auto intersectRayBox(Ray const& r, float tmin, float& tEnter, float& tExit,
                       Vec3f& entryNormal) const -> bool;

  auto addRayTreeTraversal(int i, int j, int k, Vec3f const& entryNormal,
                           int step) const -> void;
  auto addRayTreeHitCell(int i, int j, int k, int step) const -> void;
  auto paintVoxelFace(Vec3f const& a, Vec3f const& b, Vec3f const& c,
                      Vec3f const& d, Vec3f const& normal, int count) const -> void;
  auto getOccupiedCount() const -> int;

  BoundingBox* sceneBounds{};
  int nx{};
  int ny{};
  int nz{};
  float dx{};
  float dy{};
  float dz{};
  Object3DVector* cells{};
  Object3DVector* infiniteObjects{};
  Object3DVector* gridWrappers{};
  PhongMaterial** densityMaterials{};
};
