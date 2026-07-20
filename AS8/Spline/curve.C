#include "curve.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include "gl_headers.h"
#include "arg_parser.h"
#include "triangle_mesh.h"

namespace {

auto MakeBasisMatrix(float const m[4][4]) -> Matrix
{
  Matrix B;
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      B.Set(x, y, m[y][x]);
    }
  }
  return B;
}

auto ConvertControlPoints(Vec3f const src[4], Vec3f dst[4], Matrix const& srcBasis,
                          Matrix const& dstBasis) -> void {
  auto G = GeometryMatrixFromControlPoints(src);
  auto dstInv = dstBasis;
  dstInv.Inverse();
  auto Gdst = G * srcBasis * dstInv;
  ControlPointsFromGeometryMatrix(Gdst, dst);
}

}  // namespace

auto GetBezierBasisMatrix() -> Matrix
{
  static float const m[4][4] = {
      {-1, 3, -3, 1},
      {3, -6, 3, 0},
      {-3, 3, 0, 0},
      {1, 0, 0, 0},
  };
  return MakeBasisMatrix(m);
}

auto GetBSplineBasisMatrix() -> Matrix
{
  static float const m[4][4] = {
      {-1.0f / 6.0f, 3.0f / 6.0f, -3.0f / 6.0f, 1.0f / 6.0f},
      {3.0f / 6.0f, -6.0f / 6.0f, 0.0f / 6.0f, 4.0f / 6.0f},
      {-3.0f / 6.0f, 3.0f / 6.0f, 3.0f / 6.0f, 1.0f / 6.0f},
      {1.0f / 6.0f, 0.0f / 6.0f, 0.0f / 6.0f, 0.0f / 6.0f},
  };
  return MakeBasisMatrix(m);
}

auto GeometryMatrixFromControlPoints(Vec3f const pts[4]) -> Matrix
{
  Matrix G;
  G.Clear();
  for (auto col = 0; col < 4; col++) {
    G.Set(col, 0, pts[col].x());
    G.Set(col, 1, pts[col].y());
    G.Set(col, 2, pts[col].z());
    G.Set(col, 3, 1.0f);
  }
  return G;
}

auto ControlPointsFromGeometryMatrix(Matrix const& G, Vec3f pts[4]) -> void
{
  for (auto col = 0; col < 4; col++) {
    pts[col] = Vec3f(G.Get(col, 0), G.Get(col, 1), G.Get(col, 2));
  }
}

auto EvaluateCubicCurve(Vec3f const pts[4], Matrix const& basis, float t) -> Vec3f
{
  auto G = GeometryMatrixFromControlPoints(pts);
  auto GB = G * basis;
  Vec4f T(t * t * t, t * t, t, 1.0f);
  GB.Transform(T);
  return Vec3f(T.x(), T.y(), T.z());
}

auto ConvertBezierControlPointsToBSpline(Vec3f const bezier[4], Vec3f bspline[4]) -> void
{
  ConvertControlPoints(bezier, bspline, GetBezierBasisMatrix(), GetBSplineBasisMatrix());
#if DEBUG_CURVE
  DebugVerifyCurveConversion(bezier, bspline, GetBezierBasisMatrix(), GetBSplineBasisMatrix(),
                             "Bezier -> BSpline");
#endif
}

auto ConvertBSplineControlPointsToBezier(Vec3f const bspline[4], Vec3f bezier[4]) -> void
{
  ConvertControlPoints(bspline, bezier, GetBSplineBasisMatrix(), GetBezierBasisMatrix());
#if DEBUG_CURVE
  DebugVerifyCurveConversion(bspline, bezier, GetBSplineBasisMatrix(), GetBezierBasisMatrix(),
                             "BSpline -> Bezier");
#endif
}

#if DEBUG_CURVE
auto DebugVerifyCurveConversion(Vec3f const src[4], Vec3f const dst[4], Matrix const& srcBasis,
                                Matrix const& dstBasis, char const* label) -> void {
  ::printf("[DEBUG_CURVE] %s conversion check:\n", label);
  for (auto i = 0; i < 4; i++) {
    ::printf("  dst[%d] = (%g, %g, %g)\n", i, dst[i].x(), dst[i].y(), dst[i].z());
  }
  static float const samples[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
  for (auto i = 0; i < 5; i++) {
    auto t = samples[i];
    auto pSrc = EvaluateCubicCurve(src, srcBasis, t);
    auto pDst = EvaluateCubicCurve(dst, dstBasis, t);
    auto err = (pSrc - pDst).Length();
    ::printf("  t=%g  src=(%g,%g)  dst=(%g,%g)  err=%g\n", t, pSrc.x(), pSrc.y(), pDst.x(),
             pDst.y(), err);
  }
}
#endif

Curve::Curve(int _num_vertices)
{
  num_vertices = _num_vertices;
  vertices = new Vec3f[num_vertices];
}

Curve::~Curve()
{
  delete[] vertices;
}

auto Curve::getVertex(int i) -> Vec3f
{
  assert(i >= 0 && i < num_vertices);
  return vertices[i];
}

auto Curve::set(int i, Vec3f v) -> void
{
  assert(i >= 0 && i < num_vertices);
  vertices[i] = v;
}

auto Curve::moveControlPoint(int selectedPoint, float x, float y) -> void
{
  assert(selectedPoint >= 0 && selectedPoint < num_vertices);
  vertices[selectedPoint].Set(x, y, vertices[selectedPoint].z());
}

auto Curve::insertControlPoint(int index, Vec3f v) -> void
{
  auto* new_vertices = new Vec3f[num_vertices + 1];
  for (auto i = 0; i < index; i++) {
    new_vertices[i] = vertices[i];
  }
  new_vertices[index] = v;
  for (auto i = index; i < num_vertices; i++) {
    new_vertices[i + 1] = vertices[i];
  }
  delete[] vertices;
  vertices = new_vertices;
  num_vertices++;
}

auto Curve::removeControlPoint(int index) -> void
{
  assert(index >= 0 && index < num_vertices);
  auto* new_vertices = new Vec3f[num_vertices - 1];
  for (auto i = 0; i < index; i++) {
    new_vertices[i] = vertices[i];
  }
  for (auto i = index + 1; i < num_vertices; i++) {
    new_vertices[i - 1] = vertices[i];
  }
  delete[] vertices;
  vertices = new_vertices;
  num_vertices--;
}

auto Curve::addControlPoint(int selectedPoint, float x, float y) -> void
{
  if (!allowAddControlPoints()) {
    return;
  }
  insertControlPoint(selectedPoint, Vec3f(x, y, 0));
}

auto Curve::deleteControlPoint(int selectedPoint) -> void
{
  if (!allowDeleteControlPoints()) {
    return;
  }
  if (num_vertices <= 4)
  {
    return;
  }
  removeControlPoint(selectedPoint);
}

auto Curve::evaluateSegment(int segment, float t) const -> Vec3f
{
  Vec3f pts[4];
  getSegmentControlPoints(segment, pts);
  return EvaluateCubicCurve(pts, getSegmentBasis(), t);
}

auto Curve::numSegments() const -> int
{
  return getNumSegments();
}

auto Curve::evaluateAlongCurve(float u) const -> Vec3f
{
  auto numSegs = getNumSegments();
  if (u <= 0.0f)
  {
    return evaluateSegment(0, 0.0f);
  }
  if (u >= 1.0f)
  {
    return evaluateSegment(numSegs - 1, 1.0f);
  }

  auto scaled = u * numSegs;
  auto segment = static_cast<int>(scaled);
  if (segment >= numSegs)
  {
    segment = numSegs - 1;
  }
  auto t = scaled - segment;
  return evaluateSegment(segment, t);
}

auto Curve::writeControlPoints(FILE* file, char const* type) const -> void
{
  ::fprintf(file, "%s\n", type);
  ::fprintf(file, "num_vertices %d\n", num_vertices);
  for (auto i = 0; i < num_vertices; i++) {
    ::fprintf(file, "%g %g %g\n", vertices[i].x(), vertices[i].y(), vertices[i].z());
  }
}

auto Curve::Paint(ArgParser* args) -> void
{
  auto tess = args->curve_tessellation;
  if (tess < 1)
  {
    tess = 1;
  }

  ::glColor3f(0.5f, 0.5f, 0.5f);
  ::glLineWidth(1);
  ::glBegin(GL_LINES);
  for (auto i = 0; i < num_vertices - 1; i++) {
    auto v1 = vertices[i];
    auto v2 = vertices[i + 1];
    ::glVertex3f(v1.x(), v1.y(), v1.z());
    ::glVertex3f(v2.x(), v2.y(), v2.z());
  }
  ::glEnd();

  ::glColor3f(1.0f, 1.0f, 0.0f);
  ::glPointSize(5);
  ::glBegin(GL_POINTS);
  for (auto i = 0; i < num_vertices; i++) {
    ::glVertex3f(vertices[i].x(), vertices[i].y(), vertices[i].z());
  }
  ::glEnd();

  ::glColor3f(0.0f, 1.0f, 1.0f);
  ::glLineWidth(2);
  ::glBegin(GL_LINE_STRIP);
  auto segments = getNumSegments();
  for (auto s = 0; s < segments; s++) {
    for (auto i = 0; i <= tess; i++) {
      if (s > 0 && i == 0)
      {
        continue;
      }
      auto t = static_cast<float>(i) / static_cast<float>(tess);
      auto p = evaluateSegment(s, t);
      ::glVertex3f(p.x(), p.y(), p.z());
    }
  }
  ::glEnd();
}

auto Curve::OutputTriangles(ArgParser* args) -> TriangleMesh*
{
  return new TriangleMesh(0, 0);
}
