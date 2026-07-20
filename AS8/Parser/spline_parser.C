#include <cstdio>
#include <cstring>
#include <cmath>
#include <cassert>

#include "spline_parser.h"
#include "spline.h"
#include "curve.h"
#include "bezier_curve.h"
#include "bspline_curve.h"
#include "surface.h"
#include "surface_of_revolution.h"
#include "bezier_patch.h"
#include "triangle_mesh.h"
#include "arg_parser.h"

namespace {

auto DistanceToLineSegment(Vec2f a, Vec2f b, Vec2f pt) -> float {
  auto dir = b;
  dir -= a;
  auto va = pt;
  va -= a;
  auto vb = pt;
  vb -= b;
  auto denom = dir.Dot2(dir);
  if (denom < 0.00001f) {
    return va.Length();
  }
  auto t = dir.Dot2(va) / denom;
  auto proj = dir;
  proj *= t;
  proj += a;
  proj -= pt;
  if (t <= 0) {
    return va.Length();
  }
  if (t >= 1) {
    return vb.Length();
  }
  return proj.Length();
}

}  // namespace

SplineParser::SplineParser(char const* spline_file) {
  assert(spline_file != nullptr);
  file = ::fopen(spline_file, "r");
  assert(file != nullptr);
  char token[MAX_PARSER_TOKEN_LENGTH];

  getToken(token);
  assert(!::strcmp(token, "num_splines"));
  num_splines = readInt();
  splines = new Spline*[num_splines];

  for (auto i = 0; i < num_splines; i++) {
    auto* s = ParseSpline();
    assert(s != nullptr);
    splines[i] = s;
  }

  ::fclose(file);
}

SplineParser::~SplineParser() {
  for (auto i = 0; i < num_splines; i++) {
    delete splines[i];
  }
  delete[] splines;
}

auto SplineParser::ParseSpline() -> Spline* {
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  Spline* answer = nullptr;
  if (!::strcmp(token, "bezier")) {
    answer = ParseBezierCurve();
  } else if (!::strcmp(token, "bspline")) {
    answer = ParseBSplineCurve();
  } else if (!::strcmp(token, "surface_of_revolution")) {
    answer = ParseSurfaceOfRevolution();
  } else if (!::strcmp(token, "bezier_patch")) {
    answer = ParseBezierPatch();
  } else {
    ::printf("ERROR unknown spline type %s\n", token);
    assert(0);
  }
  return answer;
}

auto SplineParser::ParseBezierCurve() -> Curve* {
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "num_vertices"));
  auto num_vertices = readInt();
  assert(num_vertices >= 4);
  auto* answer = new BezierCurve(num_vertices);
  for (auto i = 0; i < num_vertices; i++) {
    auto v = readVec3f();
    answer->set(i, v);
  }
  return answer;
}

auto SplineParser::ParseBSplineCurve() -> Curve* {
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "num_vertices"));
  auto num_vertices = readInt();
  assert(num_vertices >= 4);
  auto* answer = new BSplineCurve(num_vertices);
  for (auto i = 0; i < num_vertices; i++) {
    auto v = readVec3f();
    answer->set(i, v);
  }
  return answer;
}

auto SplineParser::ParseSurfaceOfRevolution() -> Surface* {
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  Curve* c = nullptr;
  if (!::strcmp(token, "bezier")) {
    c = ParseBezierCurve();
  } else if (!::strcmp(token, "bspline")) {
    c = ParseBSplineCurve();
  } else {
    ::printf("ERROR unknown curve type %s\n", token);
  }
  assert(c != nullptr);
  return new SurfaceOfRevolution(c);
}

auto SplineParser::ParseBezierPatch() -> Surface* {
  auto* answer = new BezierPatch();
  for (auto i = 0; i < 16; i++) {
    auto v = readVec3f();
    answer->set(i, v);
  }
  return answer;
}

auto SplineParser::Pick(float x, float y, float epsilon, Spline*& selected_curve,
                          int& selected_control_point) -> void {
  selected_curve = nullptr;
  auto distance = epsilon;
  for (auto i = 0; i < num_splines; i++) {
    auto* s = getSpline(i);
    for (auto j = 0; j < s->getNumVertices(); j++) {
      auto v = s->getVertex(j);
      auto dx = v.x() - x;
      auto dy = v.y() - y;
      auto d = ::sqrt(dx * dx + dy * dy);
      if (d < distance) {
        distance = d;
        selected_curve = s;
        selected_control_point = j;
      }
    }
  }
}

auto SplineParser::PickEdge(float x, float y, float epsilon, Spline*& selected_curve,
                            int& selected_control_point) -> void {
  selected_curve = nullptr;
  auto distance = epsilon;
  for (auto i = 0; i < num_splines; i++) {
    auto* s = getSpline(i);
    for (auto j = 1; j < s->getNumVertices(); j++) {
      auto v1 = s->getVertex(j - 1);
      auto v2 = s->getVertex(j);
      auto d = DistanceToLineSegment(Vec2f(v1.x(), v1.y()), Vec2f(v2.x(), v2.y()), Vec2f(x, y));
      if (d < distance) {
        distance = d;
        selected_curve = s;
        selected_control_point = j;
      }
    }
  }
}

auto SplineParser::SaveBezier(ArgParser* args) -> void {
  if (args->output_bezier_file == nullptr) {
    return;
  }
  auto* out = ::fopen(args->output_bezier_file, "w");
  assert(out != nullptr);
  ::fprintf(out, "num_splines %d\n", getNumSplines());
  for (auto i = 0; i < getNumSplines(); i++) {
    getSpline(i)->OutputBezier(out);
  }
  ::fclose(out);
}

auto SplineParser::SaveBSpline(ArgParser* args) -> void {
  if (args->output_bspline_file == nullptr) {
    return;
  }
  auto* out = ::fopen(args->output_bspline_file, "w");
  assert(out != nullptr);
  ::fprintf(out, "num_splines %d\n", getNumSplines());
  for (auto i = 0; i < getNumSplines(); i++) {
    getSpline(i)->OutputBSpline(out);
  }
  ::fclose(out);
}

auto SplineParser::SaveTriangles(ArgParser* args) -> void {
  if (args->output_file == nullptr) {
    return;
  }
  TriangleMesh mesh(0, 0);
  for (auto i = 0; i < getNumSplines(); i++) {
    auto* m2 = getSpline(i)->OutputTriangles(args);
    mesh.Merge(*m2);
    delete m2;
  }
  auto* out = ::fopen(args->output_file, "w");
  assert(out != nullptr);
  mesh.Output(out);
  ::fclose(out);
}

auto SplineParser::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) -> int {
  assert(file != nullptr);
  auto success = ::fscanf(file, "%s ", token);
  if (success == EOF) {
    token[0] = '\0';
    return 0;
  }
  return 1;
}

auto SplineParser::readVec3f() -> Vec3f {
  float x{};
  float y{};
  float z{};
  auto count = ::fscanf(file, "%f %f %f", &x, &y, &z);
  if (count != 3) {
    ::printf("Error trying to read 3 floats to make a Vec3f\n");
    assert(0);
  }
  return Vec3f(x, y, z);
}

auto SplineParser::readVec2f() -> Vec2f {
  float u{};
  float v{};
  auto count = ::fscanf(file, "%f %f", &u, &v);
  if (count != 2) {
    ::printf("Error trying to read 2 floats to make a Vec2f\n");
    assert(0);
  }
  return Vec2f(u, v);
}

auto SplineParser::readFloat() -> float {
  float answer{};
  auto count = ::fscanf(file, "%f", &answer);
  if (count != 1) {
    ::printf("Error trying to read 1 float\n");
    assert(0);
  }
  return answer;
}

auto SplineParser::readInt() -> int {
  int answer{};
  auto count = ::fscanf(file, "%d", &answer);
  if (count != 1) {
    ::printf("Error trying to read 1 int\n");
    assert(0);
  }
  return answer;
}
