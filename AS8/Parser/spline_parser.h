#pragma once

#include <cassert>
#include <cstdio>

#include "vectors.h"

struct Spline;
struct Curve;
struct Surface;
struct ArgParser;

constexpr auto MAX_PARSER_TOKEN_LENGTH = 100;

struct SplineParser final {
  SplineParser(char const* file);
  ~SplineParser();

  auto getNumSplines() const -> int { return num_splines; }
  auto getSpline(int i) const -> Spline* {
    assert(i >= 0 && i < num_splines);
    return splines[i];
  }

  auto SaveBezier(ArgParser* args) -> void;
  auto SaveBSpline(ArgParser* args) -> void;
  auto SaveTriangles(ArgParser* args) -> void;

  auto Pick(float x, float y, float epsilon, Spline*& selected_spline,
            int& selected_control_point) -> void;
  auto PickEdge(float x, float y, float epsilon, Spline*& selected_spline,
                int& selected_control_point) -> void;

  SplineParser() { assert(0); }

  auto ParseSpline() -> Spline*;
  auto ParseBezierCurve() -> Curve*;
  auto ParseBSplineCurve() -> Curve*;
  auto ParseSurfaceOfRevolution() -> Surface*;
  auto ParseBezierPatch() -> Surface*;

  auto getToken(char token[MAX_PARSER_TOKEN_LENGTH]) -> int;
  auto readVec3f() -> Vec3f;
  auto readVec2f() -> Vec2f;
  auto readFloat() -> float;
  auto readInt() -> int;

  int num_splines{};
  Spline** splines{};
  FILE* file{};
};
