#include "gl_headers.h"
#include "curve.h"
#include "arg_parser.h"
#include "triangle_mesh.h"

#include <stdio.h>
#include <string.h>

// 基矩阵（幂基 [t^3 t^2 t^1]^T）
static Matrix MakeBasisMatrix(const float m[4][4]) {
  Matrix B;
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      B.Set(x, y, m[y][x]);
  return B;
}

// 三次贝塞尔基矩阵
Matrix GetBezierBasisMatrix() {
  static const float m[4][4] = {
    { -1,  3, -3,  1 },
    {  3, -6,  3,  0 },
    { -3,  3,  0,  0 },
    {  1,  0,  0,  0 }
  };
  return MakeBasisMatrix(m);
}

// 三次均匀 B 样条基矩阵（Q = G·B·T，T=[t^3,t^2,t,1]^T）
// 此处控制点为列，故使用 M^T
Matrix GetBSplineBasisMatrix() {
  static const float m[4][4] = {
    { -1.0f/6.0f,  3.0f/6.0f, -3.0f/6.0f,  1.0f/6.0f },
    {  3.0f/6.0f, -6.0f/6.0f,  0.0f/6.0f,  4.0f/6.0f },
    { -3.0f/6.0f,  3.0f/6.0f,  3.0f/6.0f,  1.0f/6.0f },
    {  1.0f/6.0f,  0.0f/6.0f,  0.0f/6.0f,  0.0f/6.0f }
  };
  return MakeBasisMatrix(m);
}

// 从控制点获取控制点矩阵
Matrix GeometryMatrixFromControlPoints(const Vec3f pts[4]) {
  Matrix G;
  G.Clear();
  for (int col = 0; col < 4; col++) {
    G.Set(col, 0, pts[col].x());
    G.Set(col, 1, pts[col].y());
    G.Set(col, 2, pts[col].z());
    G.Set(col, 3, 1.0f);
  }
  return G;
}

// 从控制点矩阵获取控制点
void ControlPointsFromGeometryMatrix(const Matrix &G, Vec3f pts[4]) {
  for (int col = 0; col < 4; col++)
    pts[col] = Vec3f(G.Get(col, 0), G.Get(col, 1), G.Get(col, 2));
}

// 三次样条曲线求值
Vec3f EvaluateCubicCurve(const Vec3f pts[4], const Matrix &basis, float t) {
  // 公式：P(t) = G * basis * T（G: 控制点矩阵，basis: 基矩阵，T: 参数矩阵）
  Matrix G = GeometryMatrixFromControlPoints(pts);
  Matrix GB = G * basis;
  Vec4f T(t * t * t, t * t, t, 1.0f);
  GB.Transform(T);
  return Vec3f(T.x(), T.y(), T.z());
}

// G_dst = G_src * B_src * B_dst^{-1}
static void ConvertControlPoints(const Vec3f src[4], Vec3f dst[4],
                                 const Matrix &srcBasis, const Matrix &dstBasis) {
  Matrix G = GeometryMatrixFromControlPoints(src);
  Matrix dstInv = dstBasis;
  dstInv.Inverse();
  Matrix Gdst = G * srcBasis * dstInv;
  ControlPointsFromGeometryMatrix(Gdst, dst);
}

// 贝塞尔控制点转换为B样条控制点
void ConvertBezierControlPointsToBSpline(const Vec3f bezier[4], Vec3f bspline[4]) {
  ConvertControlPoints(bezier, bspline, GetBezierBasisMatrix(), GetBSplineBasisMatrix());
#if DEBUG_CURVE
  DebugVerifyCurveConversion(bezier, bspline,
                             GetBezierBasisMatrix(), GetBSplineBasisMatrix(),
                             "Bezier -> BSpline");
#endif
}

// B样条控制点转换为贝塞尔控制点
void ConvertBSplineControlPointsToBezier(const Vec3f bspline[4], Vec3f bezier[4]) {
  ConvertControlPoints(bspline, bezier, GetBSplineBasisMatrix(), GetBezierBasisMatrix());
#if DEBUG_CURVE
  DebugVerifyCurveConversion(bspline, bezier,
                             GetBSplineBasisMatrix(), GetBezierBasisMatrix(),
                             "BSpline -> Bezier");
#endif
}

#if DEBUG_CURVE
// 在贝塞尔/B样条互转时打印采样对比
void DebugVerifyCurveConversion(const Vec3f src[4], const Vec3f dst[4],
                                const Matrix &srcBasis, const Matrix &dstBasis,
                                const char *label) {
  printf("[DEBUG_CURVE] %s conversion check:\n", label);
  for (int i = 0; i < 4; i++)
    printf("  dst[%d] = (%g, %g, %g)\n", i, dst[i].x(), dst[i].y(), dst[i].z());
  const float samples[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
  for (int i = 0; i < 5; i++) {
    float t = samples[i];
    Vec3f pSrc = EvaluateCubicCurve(src, srcBasis, t);
    Vec3f pDst = EvaluateCubicCurve(dst, dstBasis, t);
    float err = (pSrc - pDst).Length();
    printf("  t=%g  src=(%g,%g)  dst=(%g,%g)  err=%g\n",
           t, pSrc.x(), pSrc.y(), pDst.x(), pDst.y(), err);
  }
}
#endif

// ====================================================================
// Curve

Curve::Curve(int _num_vertices) {
  num_vertices = _num_vertices;
  vertices = new Vec3f[num_vertices];
}

Curve::~Curve() {
  delete [] vertices;
}

Vec3f Curve::getVertex(int i) {
  assert(i >= 0 && i < num_vertices);
  return vertices[i];
}

void Curve::set(int i, Vec3f v) {
  assert(i >= 0 && i < num_vertices);
  vertices[i] = v;
}

void Curve::moveControlPoint(int selectedPoint, float x, float y) {
  assert(selectedPoint >= 0 && selectedPoint < num_vertices);
  vertices[selectedPoint].Set(x, y, vertices[selectedPoint].z());
}

void Curve::insertControlPoint(int index, Vec3f v) {
  Vec3f *new_vertices = new Vec3f[num_vertices + 1];
  for (int i = 0; i < index; i++)
    new_vertices[i] = vertices[i];
  new_vertices[index] = v;
  for (int i = index; i < num_vertices; i++)
    new_vertices[i + 1] = vertices[i];
  delete [] vertices;
  vertices = new_vertices;
  num_vertices++;
}

void Curve::removeControlPoint(int index) {
  assert(index >= 0 && index < num_vertices);
  Vec3f *new_vertices = new Vec3f[num_vertices - 1];
  for (int i = 0; i < index; i++)
    new_vertices[i] = vertices[i];
  for (int i = index + 1; i < num_vertices; i++)
    new_vertices[i - 1] = vertices[i];
  delete [] vertices;
  vertices = new_vertices;
  num_vertices--;
}

void Curve::addControlPoint(int selectedPoint, float x, float y) {
  if (!allowAddControlPoints())
    return;
  insertControlPoint(selectedPoint, Vec3f(x, y, 0));
}

void Curve::deleteControlPoint(int selectedPoint) {
  if (!allowDeleteControlPoints())
    return;
  if (num_vertices <= 4)
    return;
  removeControlPoint(selectedPoint);
}

// 分段求值
Vec3f Curve::evaluateSegment(int segment, float t) const {
  Vec3f pts[4];
  getSegmentControlPoints(segment, pts);
  return EvaluateCubicCurve(pts, getSegmentBasis(), t);
}

int Curve::numSegments() const {
  return getNumSegments();
}

// 将 u（u ∈ [0,1]）映射到各段的局部参数 t 并求值
Vec3f Curve::evaluateAlongCurve(float u) const {
  int numSegs = getNumSegments();
  if (u <= 0.0f)  // u=0 映射到第 0 段 t=0
    return evaluateSegment(0, 0.0f);
  if (u >= 1.0f)  // u=1 映射到第 numSegs-1 段 t=1
    return evaluateSegment(numSegs - 1, 1.0f);

  float scaled = u * numSegs;
  int segment = (int)scaled;
  if (segment >= numSegs)  // segment 超出范围，映射到第 numSegs-1 段
    segment = numSegs - 1;
  float t = scaled - segment;  // 第 segment 段上的局部参数 t
  return evaluateSegment(segment, t);  // 求值
}

void Curve::writeControlPoints(FILE *file, const char *type) const {
  fprintf(file, "%s\n", type);
  fprintf(file, "num_vertices %d\n", num_vertices);
  for (int i = 0; i < num_vertices; i++)
    fprintf(file, "%g %g %g\n",
            vertices[i].x(), vertices[i].y(), vertices[i].z());
}

// 可视化
void Curve::Paint(ArgParser *args) {
  int tess = args->curve_tessellation;
  if (tess < 1) tess = 1;

  // 控制多边形
  glColor3f(0.5f, 0.5f, 0.5f);
  glLineWidth(1);
  glBegin(GL_LINES);
  for (int i = 0; i < num_vertices - 1; i++) {
    Vec3f v1 = vertices[i];
    Vec3f v2 = vertices[i + 1];
    glVertex3f(v1.x(), v1.y(), v1.z());
    glVertex3f(v2.x(), v2.y(), v2.z());
  }
  glEnd();

  // 控制点
  glColor3f(1.0f, 1.0f, 0.0f);
  glPointSize(5);
  glBegin(GL_POINTS);
  for (int i = 0; i < num_vertices; i++)
    glVertex3f(vertices[i].x(), vertices[i].y(), vertices[i].z());
  glEnd();

  // 样条曲线
  glColor3f(0.0f, 1.0f, 1.0f);
  glLineWidth(2);
  glBegin(GL_LINE_STRIP);
  int numSegments = getNumSegments();
  for (int s = 0; s < numSegments; s++) {
    for (int i = 0; i <= tess; i++) {
      if (s > 0 && i == 0)
        continue;
      float t = (float)i / (float)tess;
      Vec3f p = evaluateSegment(s, t);
      glVertex3f(p.x(), p.y(), p.z());
    }
  }
  glEnd();
}

TriangleMesh* Curve::OutputTriangles(ArgParser *args) {
  return new TriangleMesh(0, 0);
}
