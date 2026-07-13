#include "gl_headers.h"
#include "surface_of_revolution.h"
#include "curve.h"
#include "arg_parser.h"
#include "triangle_mesh.h"

#include <stdio.h>
#include <math.h>
#include <assert.h>

#define DEBUG_SURFACE 0

// 轮廓点 (x, y) 绕 y 轴旋转： (x·cosθ, y, x·sinθ)
static Vec3f RevolveProfilePoint(const Vec3f &profile, float theta) {
  float x = profile.x();
  float y = profile.y();
  return Vec3f(x * cosf(theta), y, x * sinf(theta));
}

#if DEBUG_SURFACE
static void DebugPrintRevolutionMesh(int u_tess, int v_tess,
                                     const Vec3f &profileStart, const Vec3f &profileEnd) {
  printf("[DEBUG_SURFACE] SurfaceOfRevolution mesh:\n");
  printf("  u_tess=%d  v_tess=%d  vertices=%d  triangles=%d\n",
         u_tess, v_tess, (u_tess + 1) * (v_tess + 1), u_tess * v_tess * 2); // 顶点数=(u_tess+1)*(v_tess+1)，三角形数=u_tess*v_tess*2
  printf("  profile u=0   -> (%g, %g, %g)\n",
         profileStart.x(), profileStart.y(), profileStart.z()); // 轮廓起点
  printf("  profile u=1   -> (%g, %g, %g)\n",
         profileEnd.x(), profileEnd.y(), profileEnd.z()); // 轮廓终点
}
#endif

SurfaceOfRevolution::SurfaceOfRevolution(Curve *profile) {
  profile_curve = profile;
}

SurfaceOfRevolution::~SurfaceOfRevolution() {
  delete profile_curve;
}

Vec3f SurfaceOfRevolution::getVertex(int i) {
  assert(profile_curve != NULL);
  return profile_curve->getVertex(i);
}

int SurfaceOfRevolution::getNumVertices() {
  assert(profile_curve != NULL);
  return profile_curve->getNumVertices();
}

void SurfaceOfRevolution::moveControlPoint(int selectedPoint, float x, float y) {
  assert(profile_curve != NULL);
  profile_curve->moveControlPoint(selectedPoint, x, y);
}

void SurfaceOfRevolution::addControlPoint(int selectedPoint, float x, float y) {
  assert(profile_curve != NULL);
  profile_curve->addControlPoint(selectedPoint, x, y);
}

void SurfaceOfRevolution::deleteControlPoint(int selectedPoint) {
  assert(profile_curve != NULL);
  profile_curve->deleteControlPoint(selectedPoint);
}

void SurfaceOfRevolution::Paint(ArgParser *args) {
  // 绘制 2D 轮廓曲线
  if (profile_curve != NULL)
    profile_curve->Paint(args);

  if (profile_curve == NULL)
    return;

  int curveTess = args->curve_tessellation;  // 轮廓分段数
  if (curveTess < 1) curveTess = 1;
  int numSamples = 8;  // 轮廓采样点数
  int revSamples = 24;  // 旋转截面圆采样点数
  const float twoPi = 2.0f * 3.14159265f;  // 2π

  glColor3f(0.2f, 0.8f, 0.2f);
  glLineWidth(1);
  // 绘制轮廓采样点
  for (int s = 0; s <= numSamples; s++) {
    float u = (float)s / (float)numSamples;
    Vec3f profile = profile_curve->evaluateAlongCurve(u);
    glBegin(GL_LINE_LOOP);
    for (int j = 0; j < revSamples; j++) {
      float theta = twoPi * (float)j / (float)revSamples;
      Vec3f v = RevolveProfilePoint(profile, theta);
      glVertex3f(v.x(), v.y(), v.z());
    }
    glEnd();
  }
}

void SurfaceOfRevolution::OutputBezier(FILE *file) {
  fprintf(file, "surface_of_revolution\n");
  profile_curve->OutputBezier(file);
}

void SurfaceOfRevolution::OutputBSpline(FILE *file) {
  fprintf(file, "surface_of_revolution\n");
  profile_curve->OutputBSpline(file);
}

// 输出三角网格
TriangleMesh* SurfaceOfRevolution::OutputTriangles(ArgParser *args) {
  assert(profile_curve != NULL);

  int curveTess = args->curve_tessellation;  // 轮廓分段数
  int revTess = args->revolution_tessellation;  // 旋转截面圆分段数
  if (curveTess < 1) curveTess = 1;
  if (revTess < 1) revTess = 1;

  int u_tess = profile_curve->numSegments() * curveTess;  // u 方向分段数=轮廓分段数*轮廓采样点数
  int v_tess = revTess;  // v 方向分段数=旋转截面圆采样点数=旋转截面圆分段数

  TriangleNet *net = new TriangleNet(u_tess, v_tess);
  const float twoPi = 2.0f * 3.14159265f;

  // 遍历 u 方向的轮廓采样点
  for (int i = 0; i <= u_tess; i++) {
    float u = (float)i / (float)u_tess;  // 沿轮廓参数 u
    Vec3f profile = profile_curve->evaluateAlongCurve(u);
    // 遍历 v 方向的旋转截面圆采样点
    for (int j = 0; j <= v_tess; j++) {
      float theta = twoPi * (float)j / (float)v_tess;  // 沿旋转截面圆参数 θ（绕 y 轴旋转）
      net->SetVertex(i, j, RevolveProfilePoint(profile, theta));
    }
  }

#if DEBUG_SURFACE
  DebugPrintRevolutionMesh(u_tess, v_tess,
                           profile_curve->evaluateAlongCurve(0.0f),
                           profile_curve->evaluateAlongCurve(1.0f));
#endif

  return net;
}
