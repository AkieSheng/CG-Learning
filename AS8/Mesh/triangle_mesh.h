#pragma once

#include <cassert>
#include <cstdio>

#include "vectors.h"

struct TriangleMesh {
  TriangleMesh(int num_verts, int num_tris);
  ~TriangleMesh()
  {
    delete[] vertices;
    delete[] triangles;
  }

  auto SetVertex(int i, Vec3f v) -> void
  {
    assert(i >= 0 && i < num_vertices);
    vertices[i] = v;
  }
  auto SetTriangle(int i, int a, int b, int c) -> void
  {
    assert(i >= 0 && i < num_triangles);
    triangles[i * 3 + 0] = a;
    triangles[i * 3 + 1] = b;
    triangles[i * 3 + 2] = c;
  }
  auto Merge(TriangleMesh const& m) -> void;
  auto Output(FILE* file) -> void;

  TriangleMesh()
  { assert(0); }

  int num_vertices{};
  int num_triangles{};
  Vec3f* vertices{};
  int* triangles{};
};

struct TriangleNet final : TriangleMesh {
  TriangleNet(int _u_tess, int _v_tess);
  ~TriangleNet()
  { }

  auto SetVertex(int i, int j, Vec3f v) -> void
  {
    assert(i >= 0 && i <= u_tess);
    assert(j >= 0 && j <= v_tess);
    auto index = i * (v_tess + 1) + j;
    TriangleMesh::SetVertex(index, v);
  }

  TriangleNet()
  { assert(0); }

  int u_tess{};
  int v_tess{};
};
