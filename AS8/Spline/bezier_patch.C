#include <cstdio>
#include <cassert>

#include "gl_headers.h"
#include "bezier_patch.h"
#include "curve.h"
#include "arg_parser.h"
#include "triangle_mesh.h"

#define DEBUG_PATCH 0

namespace {

auto EvaluateBezierPatch(Vec3f const control[16], float s, float t) -> Vec3f {
  auto const& B = GetBezierBasisMatrix();
  Vec3f rowPoints[4];
  for (auto i = 0; i < 4; i++) {
    Vec3f pts[4];
    for (auto j = 0; j < 4; j++) {
      pts[j] = control[i * 4 + j];
    }
    rowPoints[i] = EvaluateCubicCurve(pts, B, t);
  }
  return EvaluateCubicCurve(rowPoints, B, s);
}

#if DEBUG_PATCH
auto DebugPrintPatchMesh(int tess, Vec3f const control[16]) -> void {
  ::printf("[DEBUG_PATCH] BezierPatch mesh:\n");
  ::printf("  patch_tessellation=%d  vertices=%d  triangles=%d\n", tess, (tess + 1) * (tess + 1),
           tess * tess * 2);
  auto c00 = EvaluateBezierPatch(control, 0.0f, 0.0f);
  auto c10 = EvaluateBezierPatch(control, 1.0f, 0.0f);
  auto c01 = EvaluateBezierPatch(control, 0.0f, 1.0f);
  auto c11 = EvaluateBezierPatch(control, 1.0f, 1.0f);
  ::printf("  corner (s=0,t=0) -> (%g, %g, %g)\n", c00.x(), c00.y(), c00.z());
  ::printf("  corner (s=1,t=0) -> (%g, %g, %g)\n", c10.x(), c10.y(), c10.z());
  ::printf("  corner (s=0,t=1) -> (%g, %g, %g)\n", c01.x(), c01.y(), c01.z());
  ::printf("  corner (s=1,t=1) -> (%g, %g, %g)\n", c11.x(), c11.y(), c11.z());
}
#endif

}  // namespace

BezierPatch::BezierPatch() {
  for (auto i = 0; i < 16; i++) {
    control_points[i] = Vec3f(0, 0, 0);
  }
}

BezierPatch::~BezierPatch() {}

auto BezierPatch::set(int i, Vec3f v) -> void {
  assert(i >= 0 && i < 16);
  control_points[i] = v;
}

auto BezierPatch::getVertex(int i) -> Vec3f {
  assert(i >= 0 && i < 16);
  return control_points[i];
}

auto BezierPatch::Paint(ArgParser* args) -> void {
  ::glColor3f(0.5f, 0.5f, 0.5f);
  ::glLineWidth(1);

  ::glBegin(GL_LINES);

  for (auto i = 0; i < 4; i++) {
    for (auto j = 0; j < 3; j++) {
      auto a = control_points[i * 4 + j];
      auto b = control_points[i * 4 + j + 1];
      ::glVertex3f(a.x(), a.y(), a.z());
      ::glVertex3f(b.x(), b.y(), b.z());
    }

    for (auto j = 0; j < 3; j++) {
      auto a = control_points[j * 4 + i];
      auto b = control_points[(j + 1) * 4 + i];
      ::glVertex3f(a.x(), a.y(), a.z());
      ::glVertex3f(b.x(), b.y(), b.z());
    }
  }

  ::glEnd();
  ::glColor3f(1.0f, 1.0f, 0.0f);
  ::glPointSize(4);
  ::glBegin(GL_POINTS);

  for (auto i = 0; i < 16; i++) {
    ::glVertex3f(control_points[i].x(), control_points[i].y(), control_points[i].z());
  }

  ::glEnd();

  auto tess = args->patch_tessellation;
  if (tess < 1) {
    tess = 1;
  }
  auto previewTess = tess;

  if (previewTess > 20) {
    previewTess = 20;
  }

  ::glColor3f(0.0f, 1.0f, 1.0f);
  ::glLineWidth(1);

  for (auto i = 0; i <= previewTess; i++) {
    auto s = static_cast<float>(i) / static_cast<float>(previewTess);
    ::glBegin(GL_LINE_STRIP);

    for (auto j = 0; j <= previewTess; j++) {
      auto t = static_cast<float>(j) / static_cast<float>(previewTess);
      auto p = EvaluateBezierPatch(control_points, s, t);
      ::glVertex3f(p.x(), p.y(), p.z());
    }
    ::glEnd();
  }

  for (auto j = 0; j <= previewTess; j++) {
    auto t = static_cast<float>(j) / static_cast<float>(previewTess);
    ::glBegin(GL_LINE_STRIP);

    for (auto i = 0; i <= previewTess; i++) {
      auto s = static_cast<float>(i) / static_cast<float>(previewTess);
      auto p = EvaluateBezierPatch(control_points, s, t);
      ::glVertex3f(p.x(), p.y(), p.z());
    }
    ::glEnd();
  }
}

auto BezierPatch::OutputBezier(FILE* file) -> void {
  ::fprintf(file, "bezier_patch\n");
  for (auto i = 0; i < 16; i++) {
    ::fprintf(file, "%g %g %g\n", control_points[i].x(), control_points[i].y(),
              control_points[i].z());
  }
}

auto BezierPatch::OutputBSpline(FILE* file) -> void {
  OutputBezier(file);
}

auto BezierPatch::OutputTriangles(ArgParser* args) -> TriangleMesh* {
  auto tess = args->patch_tessellation;
  if (tess < 1) {
    tess = 1;
  }

  auto* net = new TriangleNet(tess, tess);

  for (auto i = 0; i <= tess; i++) {
    auto s = static_cast<float>(i) / static_cast<float>(tess);
    for (auto j = 0; j <= tess; j++) {
      auto t = static_cast<float>(j) / static_cast<float>(tess);
      net->SetVertex(i, j, EvaluateBezierPatch(control_points, s, t));
    }
  }

  for (auto i = 0; i < tess; i++) {
    for (auto j = 0; j < tess; j++) {
      auto index = (i * tess + j) * 2;
      auto a1 = i * (tess + 1) + j;
      auto a2 = (i + 1) * (tess + 1) + j;
      auto b1 = i * (tess + 1) + (j + 1);
      auto b2 = (i + 1) * (tess + 1) + (j + 1);
      net->SetTriangle(index, a1, a2, b1);
      net->SetTriangle(index + 1, b1, a2, b2);
    }
  }

#if DEBUG_PATCH
  DebugPrintPatchMesh(tess, control_points);
#endif
  return net;
}
