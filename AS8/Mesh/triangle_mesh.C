#include "triangle_mesh.h"

TriangleMesh::TriangleMesh(int num_verts, int num_tris)
{
  num_vertices = num_verts;
  num_triangles = num_tris;
  if (num_vertices == 0)
  {
    vertices = nullptr;
  } else {
    vertices = new Vec3f[num_vertices];
  }
  if (num_triangles == 0)
  {
    triangles = nullptr;
  } else {
    triangles = new int[num_triangles * 3];
  }
}

auto TriangleMesh::Merge(TriangleMesh const& m) -> void
{
  auto new_num_vertices = num_vertices + m.num_vertices;
  auto new_num_triangles = num_triangles + m.num_triangles;
  auto* new_vertices = new Vec3f[new_num_vertices];
  auto* new_triangles = new int[new_num_triangles * 3];

  for (auto i = 0; i < num_vertices; i++) {
    new_vertices[i] = vertices[i];
  }
  for (auto i = 0; i < m.num_vertices; i++) {
    new_vertices[i + num_vertices] = m.vertices[i];
  }
  for (auto i = 0; i < num_triangles * 3; i++) {
    new_triangles[i] = triangles[i];
  }
  for (auto i = 0; i < m.num_triangles * 3; i++) {
    new_triangles[i + num_triangles * 3] = m.triangles[i] + num_vertices;
  }

  delete[] vertices;
  delete[] triangles;
  num_vertices = new_num_vertices;
  num_triangles = new_num_triangles;
  vertices = new_vertices;
  triangles = new_triangles;
}

auto TriangleMesh::Output(FILE* file) -> void
{
  for (auto i = 0; i < num_vertices; i++) {
    ::fprintf(file, "v %f %f %f\n", vertices[i].x(), vertices[i].y(), vertices[i].z());
  }
  for (auto t = 0; t < num_triangles; t++) {
    ::fprintf(file, "f %d %d %d\n", triangles[t * 3 + 0] + 1, triangles[t * 3 + 1] + 1,
              triangles[t * 3 + 2] + 1);
  }
}

TriangleNet::TriangleNet(int _u_tess, int _v_tess)
    : TriangleMesh((_u_tess + 1) * (_v_tess + 1), _u_tess * _v_tess * 2)
{
  u_tess = _u_tess;
  v_tess = _v_tess;
  for (auto i = 0; i < u_tess; i++) {
    for (auto j = 0; j < v_tess; j++) {
      auto index = (i * v_tess + j) * 2;
      auto a1 = i * (v_tess + 1) + j;
      auto a2 = (i + 1) * (v_tess + 1) + j;
      auto b1 = i * (v_tess + 1) + (j + 1);
      auto b2 = (i + 1) * (v_tess + 1) + (j + 1);
      SetTriangle(index, a1, b1, a2);
      SetTriangle(index + 1, b1, b2, a2);
    }
  }
}
