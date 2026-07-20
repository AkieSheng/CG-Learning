#include "surface_of_revolution.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include "gl_headers.h"
#include "curve.h"
#include "arg_parser.h"
#include "triangle_mesh.h"

#define DEBUG_SURFACE 0

namespace {

auto RevolveProfilePoint(Vec3f const& profile, float theta) -> Vec3f
{
  auto x = profile.x();
  auto y = profile.y();
  return Vec3f(x * ::cosf(theta), y, x * ::sinf(theta));
}

#if DEBUG_SURFACE
auto DebugPrintRevolutionMesh(int u_tess, int v_tess, Vec3f const& profileStart,
                              Vec3f const& profileEnd) -> void {
  ::printf("[DEBUG_SURFACE] SurfaceOfRevolution mesh:\n");
  ::printf("  u_tess=%d  v_tess=%d  vertices=%d  triangles=%d\n", u_tess, v_tess,
           (u_tess + 1) * (v_tess + 1), u_tess * v_tess * 2);
  ::printf("  profile u=0   -> (%g, %g, %g)\n", profileStart.x(), profileStart.y(),
           profileStart.z());
  ::printf("  profile u=1   -> (%g, %g, %g)\n", profileEnd.x(), profileEnd.y(), profileEnd.z());
}
#endif

}  // namespace

SurfaceOfRevolution::SurfaceOfRevolution(Curve* profile)
{
  profile_curve = profile;
}

SurfaceOfRevolution::~SurfaceOfRevolution()
{
  delete profile_curve;
}

auto SurfaceOfRevolution::getVertex(int i) -> Vec3f
{
  assert(profile_curve != nullptr);
  return profile_curve->getVertex(i);
}

auto SurfaceOfRevolution::getNumVertices() -> int
{
  assert(profile_curve != nullptr);
  return profile_curve->getNumVertices();
}

auto SurfaceOfRevolution::moveControlPoint(int selectedPoint, float x, float y) -> void
{
  assert(profile_curve != nullptr);
  profile_curve->moveControlPoint(selectedPoint, x, y);
}

auto SurfaceOfRevolution::addControlPoint(int selectedPoint, float x, float y) -> void
{
  assert(profile_curve != nullptr);
  profile_curve->addControlPoint(selectedPoint, x, y);
}

auto SurfaceOfRevolution::deleteControlPoint(int selectedPoint) -> void
{
  assert(profile_curve != nullptr);
  profile_curve->deleteControlPoint(selectedPoint);
}

auto SurfaceOfRevolution::Paint(ArgParser* args) -> void
{
  if (profile_curve != nullptr)
  {
    profile_curve->Paint(args);
  }

  if (profile_curve == nullptr)
  {
    return;
  }

  auto curveTess = args->curve_tessellation;
  if (curveTess < 1)
  {
    curveTess = 1;
  }
  auto numSamples = 8;
  auto revSamples = 24;
  auto const twoPi = 2.0f * 3.14159265f;

  ::glColor3f(0.2f, 0.8f, 0.2f);
  ::glLineWidth(1);
  for (auto s = 0; s <= numSamples; s++) {
    auto u = static_cast<float>(s) / static_cast<float>(numSamples);
    auto profile = profile_curve->evaluateAlongCurve(u);
    ::glBegin(GL_LINE_LOOP);
    for (auto j = 0; j < revSamples; j++) {
      auto theta = twoPi * static_cast<float>(j) / static_cast<float>(revSamples);
      auto v = RevolveProfilePoint(profile, theta);
      ::glVertex3f(v.x(), v.y(), v.z());
    }
    ::glEnd();
  }
}

auto SurfaceOfRevolution::OutputBezier(FILE* file) -> void
{
  ::fprintf(file, "surface_of_revolution\n");
  profile_curve->OutputBezier(file);
}

auto SurfaceOfRevolution::OutputBSpline(FILE* file) -> void
{
  ::fprintf(file, "surface_of_revolution\n");
  profile_curve->OutputBSpline(file);
}

auto SurfaceOfRevolution::OutputTriangles(ArgParser* args) -> TriangleMesh*
{
  assert(profile_curve != nullptr);

  auto curveTess = args->curve_tessellation;
  auto revTess = args->revolution_tessellation;
  if (curveTess < 1)
  {
    curveTess = 1;
  }
  if (revTess < 1)
  {
    revTess = 1;
  }

  auto u_tess = profile_curve->numSegments() * curveTess;
  auto v_tess = revTess;

  auto* net = new TriangleNet(u_tess, v_tess);
  auto const twoPi = 2.0f * 3.14159265f;

  for (auto i = 0; i <= u_tess; i++) {
    auto u = static_cast<float>(i) / static_cast<float>(u_tess);
    auto profile = profile_curve->evaluateAlongCurve(u);
    for (auto j = 0; j <= v_tess; j++) {
      auto theta = twoPi * static_cast<float>(j) / static_cast<float>(v_tess);
      net->SetVertex(i, j, RevolveProfilePoint(profile, theta));
    }
  }

#if DEBUG_SURFACE
  DebugPrintRevolutionMesh(u_tess, v_tess, profile_curve->evaluateAlongCurve(0.0f),
                           profile_curve->evaluateAlongCurve(1.0f));
#endif

  return net;
}
