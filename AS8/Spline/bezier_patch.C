#include "gl_headers.h"
#include "bezier_patch.h"
#include "curve.h"
#include "arg_parser.h"
#include "triangle_mesh.h"

#include <stdio.h>
#include <assert.h>

#define DEBUG_PATCH 0

// Bezier Patch 求值
static Vec3f EvaluateBezierPatch(const Vec3f control[16], float s, float t) {
  const Matrix &B = GetBezierBasisMatrix();
  Vec3f rowPoints[4];
  for (int i = 0; i < 4; i++) {
    Vec3f pts[4];
    for (int j = 0; j < 4; j++)
      pts[j] = control[i * 4 + j];  // 每行 4 控制点
    rowPoints[i] = EvaluateCubicCurve(pts, B, t);  // 用 t 沿每行 4 控制点做三次 Bezier
  }
  return EvaluateCubicCurve(rowPoints, B, s);  // 再用 s 在 4 个结果点间插值，返回 (s,t) 处的值
}


#if DEBUG_PATCH

static void DebugPrintPatchMesh(int tess, const Vec3f control[16]) {
  printf("[DEBUG_PATCH] BezierPatch mesh:\n");
  printf("  patch_tessellation=%d  vertices=%d  triangles=%d\n",
         tess, (tess + 1) * (tess + 1), tess * tess * 2);
  Vec3f c00 = EvaluateBezierPatch(control, 0.0f, 0.0f);
  Vec3f c10 = EvaluateBezierPatch(control, 1.0f, 0.0f);
  Vec3f c01 = EvaluateBezierPatch(control, 0.0f, 1.0f);
  Vec3f c11 = EvaluateBezierPatch(control, 1.0f, 1.0f);
  printf("  corner (s=0,t=0) -> (%g, %g, %g)\n", c00.x(), c00.y(), c00.z());
  printf("  corner (s=1,t=0) -> (%g, %g, %g)\n", c10.x(), c10.y(), c10.z());
  printf("  corner (s=0,t=1) -> (%g, %g, %g)\n", c01.x(), c01.y(), c01.z());
  printf("  corner (s=1,t=1) -> (%g, %g, %g)\n", c11.x(), c11.y(), c11.z());
}

#endif

BezierPatch::BezierPatch() {
  for (int i = 0; i < 16; i++)
    control_points[i] = Vec3f(0, 0, 0);
}

BezierPatch::~BezierPatch() {}

void BezierPatch::set(int i, Vec3f v) {
  assert(i >= 0 && i < 16);
  control_points[i] = v;
}

Vec3f BezierPatch::getVertex(int i) {
  assert(i >= 0 && i < 16);
  return control_points[i];
}

void BezierPatch::Paint(ArgParser *args) {
  // 绘制 4×4 控制点网格
  glColor3f(0.5f, 0.5f, 0.5f);
  glLineWidth(1);

  glBegin(GL_LINES);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      Vec3f a = control_points[i * 4 + j];
      Vec3f b = control_points[i * 4 + j + 1];
      glVertex3f(a.x(), a.y(), a.z());
      glVertex3f(b.x(), b.y(), b.z());
    }

    for (int j = 0; j < 3; j++) {
      Vec3f a = control_points[j * 4 + i];
      Vec3f b = control_points[(j + 1) * 4 + i];
      glVertex3f(a.x(), a.y(), a.z());
      glVertex3f(b.x(), b.y(), b.z());
    }
  }

  glEnd();
  glColor3f(1.0f, 1.0f, 0.0f);
  glPointSize(4);
  glBegin(GL_POINTS);

  for (int i = 0; i < 16; i++)
    glVertex3f(control_points[i].x(), control_points[i].y(), control_points[i].z());

  glEnd();

  int tess = args->patch_tessellation;  // 采样点数
  if (tess < 1) tess = 1;
  int previewTess = tess;

  if (previewTess > 20)
    previewTess = 20;

  glColor3f(0.0f, 1.0f, 1.0f);
  glLineWidth(1);

  // 绘制水平控制线
  for (int i = 0; i <= previewTess; i++) {
    float s = (float)i / (float)previewTess;
    glBegin(GL_LINE_STRIP);

    for (int j = 0; j <= previewTess; j++) {
      float t = (float)j / (float)previewTess;
      Vec3f p = EvaluateBezierPatch(control_points, s, t);
      glVertex3f(p.x(), p.y(), p.z());
    }
    glEnd();
  }

  // 绘制垂直控制线
  for (int j = 0; j <= previewTess; j++) {
    float t = (float)j / (float)previewTess;
    glBegin(GL_LINE_STRIP);

    for (int i = 0; i <= previewTess; i++) {
      float s = (float)i / (float)previewTess;
      Vec3f p = EvaluateBezierPatch(control_points, s, t);
      glVertex3f(p.x(), p.y(), p.z());
    }
    glEnd();
  }
}



void BezierPatch::OutputBezier(FILE *file) {
  fprintf(file, "bezier_patch\n");
  for (int i = 0; i < 16; i++)
    fprintf(file, "%g %g %g\n", control_points[i].x(), control_points[i].y(), control_points[i].z());
}

void BezierPatch::OutputBSpline(FILE *file) {
  OutputBezier(file);
}

TriangleMesh* BezierPatch::OutputTriangles(ArgParser *args) {
  int tess = args->patch_tessellation;
  if (tess < 1) tess = 1;

  TriangleNet *net = new TriangleNet(tess, tess);

  // u ↔ s，v ↔ t；网格角点 (i,j) 对应参数 (s,t)
  for (int i = 0; i <= tess; i++) {
    float s = (float)i / (float)tess;
    for (int j = 0; j <= tess; j++) {
      float t = (float)j / (float)tess;
      net->SetVertex(i, j, EvaluateBezierPatch(control_points, s, t));
    }
  }

  // TriangleNet 默认绕序使 Patch 法线背向相机，这里翻转每个三角形的顶点顺序改变其面向，显示正面颜色
  for (int i = 0; i < tess; i++) {
    for (int j = 0; j < tess; j++) {
      int index = (i * tess + j) * 2;
      int a1 =  i   * (tess + 1) +  j;
      int a2 = (i + 1) * (tess + 1) +  j;
      int b1 =  i   * (tess + 1) + (j + 1);
      int b2 = (i + 1) * (tess + 1) + (j + 1);
      net->SetTriangle(index,     a1, a2, b1);
      net->SetTriangle(index + 1, b1, a2, b2);
    }
  }

#if DEBUG_PATCH
  DebugPrintPatchMesh(tess, control_points);
#endif
  return net;
}
