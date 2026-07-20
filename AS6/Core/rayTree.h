#pragma once

#include <cassert>
#include <cstdio>
#include "gl_headers.h"
#include "ray.h"
#include "material.h"

struct Segment final {
  Segment() { Clear(); }
  Segment(Ray const& ray, float tstart, float tstop) {
    if (tstart < -100)
      tstart = -100;
    if (tstop > 100)
      tstop = 100;
    a = ray.pointAtParameter(tstart);
    b = ray.pointAtParameter(tstop);
  }
  Segment(Segment const& s) {
    a = s.a;
    b = s.b;
  }
  ~Segment() {}

  auto Clear() -> void {
    a = Vec3f(0, 0, 0);
    b = Vec3f(0, 0, 0);
  }
  auto Print(char const* s) -> void {
    ::printf(" %s (%6.3f %6.3f %6.3f) -> (%6.3f %6.3f %6.3f)\n",
             s, a.x(), a.y(), a.z(), b.x(), b.y(), b.z());
  }
  auto paint() -> void {
    glVertex3f(a.x(), a.y(), a.z());
    glVertex3f(b.x(), b.y(), b.z());
  }

  Vec3f a{};
  Vec3f b{};
};

struct SegmentVector final {
  SegmentVector() {
    num_segments = 0;
    size = 10;
    segments = new Segment[size];
  }
  ~SegmentVector() { delete[] segments; }
  auto Clear() -> void { num_segments = 0; }

  auto getNumSegments() -> int { return num_segments; }
  auto getSegment(int i) -> Segment {
    assert(i >= 0 && i < num_segments);
    return segments[i];
  }

  auto addSegment(Segment const& s) -> void {
    if (size == num_segments) {
      int new_size = size * 2;
      Segment* new_segments = new Segment[new_size];
      for (int i = 0; i < size; i++)
        new_segments[i] = segments[i];
      delete[] segments;
      segments = new_segments;
      size = new_size;
    }
    segments[num_segments] = s;
    num_segments++;
  }

  Segment* segments{};
  int size{};
  int num_segments{};
};

struct CellFace final {
  CellFace() {}
  CellFace(Vec3f _a, Vec3f _b, Vec3f _c, Vec3f _d, Vec3f _normal, Material* m) {
    a = _a;
    b = _b;
    c = _c;
    d = _d;
    normal = _normal;
    material = m;
  }
  CellFace(CellFace const& f) {
    a = f.a;
    b = f.b;
    c = f.c;
    d = f.d;
    normal = f.normal;
    material = f.material;
  }
  ~CellFace() {}

  auto paint() -> void {
    material->glSetMaterial();
    glNormal3f(normal.x(), normal.y(), normal.z());
    glBegin(GL_QUADS);
    glVertex3f(a.x(), a.y(), a.z());
    glVertex3f(b.x(), b.y(), b.z());
    glVertex3f(c.x(), c.y(), c.z());
    glVertex3f(d.x(), d.y(), d.z());
    glEnd();
  }

  Vec3f a{};
  Vec3f b{};
  Vec3f c{};
  Vec3f d{};
  Vec3f normal{};
  Material* material{};
};

struct CellFaceVector final {
  CellFaceVector() {
    num_cellFaces = 0;
    size = 10;
    cellFaces = new CellFace[size];
  }
  ~CellFaceVector() { delete[] cellFaces; }
  auto Clear() -> void { num_cellFaces = 0; }

  auto getNumCellFaces() -> int { return num_cellFaces; }
  auto getCellFace(int i) -> CellFace {
    assert(i >= 0 && i < num_cellFaces);
    return cellFaces[i];
  }

  auto addCellFace(CellFace const& s) -> void {
    if (size == num_cellFaces) {
      int new_size = size * 2;
      CellFace* new_cellFaces = new CellFace[new_size];
      for (int i = 0; i < size; i++)
        new_cellFaces[i] = cellFaces[i];
      delete[] cellFaces;
      cellFaces = new_cellFaces;
      size = new_size;
    }
    cellFaces[num_cellFaces] = s;
    num_cellFaces++;
  }

  CellFace* cellFaces{};
  int size{};
  int num_cellFaces{};
};

struct RayTree final {
  static auto Activate() -> void {
    Clear();
    activated = 1;
  }
  static auto Deactivate() -> void { activated = 0; }

  static auto SetMainSegment(Ray const& ray, float tstart, float tstop) -> void {
    if (!activated)
      return;
    main_segment = Segment(ray, tstart, tstop);
  }
  static auto AddShadowSegment(Ray const& ray, float tstart, float tstop) -> void {
    if (!activated)
      return;
    shadow_segments.addSegment(Segment(ray, tstart, tstop));
  }
  static auto AddReflectedSegment(Ray const& ray, float tstart, float tstop) -> void {
    if (!activated)
      return;
    reflected_segments.addSegment(Segment(ray, tstart, tstop));
  }
  static auto AddTransmittedSegment(Ray const& ray, float tstart, float tstop) -> void {
    if (!activated)
      return;
    transmitted_segments.addSegment(Segment(ray, tstart, tstop));
  }

  static auto AddHitCellFace(Vec3f a, Vec3f b, Vec3f c, Vec3f d, Vec3f normal,
                             Material* m) -> void {
    if (!activated)
      return;
    hit_cells.addCellFace(CellFace(a, b, c, d, normal, m));
  }
  static auto AddEnteredFace(Vec3f a, Vec3f b, Vec3f c, Vec3f d, Vec3f normal,
                             Material* m) -> void {
    if (!activated)
      return;
    entered_faces.addCellFace(CellFace(a, b, c, d, normal, m));
  }

  static auto paint() -> void;
  static auto paintHitCells() -> void;
  static auto paintEnteredFaces() -> void;
  static auto Print() -> void;

private:
  static auto paintHelper(Vec4f const& m, Vec4f const& s, Vec4f const& r,
                          Vec4f const& t) -> void;
  static auto Clear() -> void {
    main_segment.Clear();
    shadow_segments.Clear();
    reflected_segments.Clear();
    transmitted_segments.Clear();
    hit_cells.Clear();
    entered_faces.Clear();
  }

  static int activated;
  static Segment main_segment;
  static SegmentVector shadow_segments;
  static SegmentVector reflected_segments;
  static SegmentVector transmitted_segments;
  static CellFaceVector hit_cells;
  static CellFaceVector entered_faces;
};
