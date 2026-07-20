#pragma once

#include "vectors.h"

#include <cmath>
#include <cassert>
#include <cstdio>

struct Matrix final
{
  Matrix() { Clear(); }
  Matrix(Matrix const& m);
  Matrix(float const* m);
  ~Matrix() {}

  auto glGet() const -> float*
  {
    auto* glMat = new float[16];
    glMat[0] = data[0][0];
    glMat[1] = data[1][0];
    glMat[2] = data[2][0];
    glMat[3] = data[3][0];
    glMat[4] = data[0][1];
    glMat[5] = data[1][1];
    glMat[6] = data[2][1];
    glMat[7] = data[3][1];
    glMat[8] = data[0][2];
    glMat[9] = data[1][2];
    glMat[10] = data[2][2];
    glMat[11] = data[3][2];
    glMat[12] = data[0][3];
    glMat[13] = data[1][3];
    glMat[14] = data[2][3];
    glMat[15] = data[3][3];
    return glMat;
  }
  auto Get(int x, int y) const -> float
  {
    assert((x >= 0) && (x < 4));
    assert((y >= 0) && (y < 4));
    return data[y][x];
  }

  auto Set(int x, int y, float v) -> void
  {
    assert((x >= 0) && (x < 4));
    assert((y >= 0) && (y < 4));
    data[y][x] = v;
  }
  auto SetToIdentity() -> void;
  auto Clear() -> void;

  auto Transpose(Matrix& m) const -> void;
  auto Transpose() -> void { Transpose(*this); }

  auto Inverse(Matrix& m, float epsilon = 1e-08f) const -> int;
  auto Inverse(float epsilon = 1e-08f) -> int { return Inverse(*this, epsilon); }

  auto operator = (Matrix const& m) -> Matrix&;
  auto operator == (Matrix const& m) const -> int;
  auto operator != (Matrix const& m) const -> int { return !(*this == m); }
  friend auto operator + (Matrix const& m1, Matrix const& m2) -> Matrix;
  friend auto operator - (Matrix const& m1, Matrix const& m2) -> Matrix;
  friend auto operator * (Matrix const& m1, Matrix const& m2) -> Matrix;
  friend auto operator * (Matrix const& m1, float f) -> Matrix;
  friend auto operator * (float f, Matrix const& m) -> Matrix { return m * f; }
  auto operator += (Matrix const& m) -> Matrix&
  {
    *this = *this + m;
    return *this;
  }
  auto operator -= (Matrix const& m) -> Matrix&
  {
    *this = *this - m;
    return *this;
  }
  auto operator *= (float const f) -> Matrix&
  {
    *this = *this * f;
    return *this;
  }
  auto operator *= (Matrix const& m) -> Matrix&
  {
    *this = *this * m;
    return *this;
  }

  static auto MakeTranslation(Vec3f const& v) -> Matrix;
  static auto MakeScale(Vec3f const& v) -> Matrix;
  static auto MakeScale(float s) -> Matrix { return MakeScale(Vec3f(s, s, s)); }
  static auto MakeXRotation(float theta) -> Matrix;
  static auto MakeYRotation(float theta) -> Matrix;
  static auto MakeZRotation(float theta) -> Matrix;
  static auto MakeAxisRotation(Vec3f const& v, float theta) -> Matrix;

  auto Transform(Vec4f& v) const -> void;
  auto Transform(Vec3f& v) const -> void
  {
    auto v2 = Vec4f(v.x(), v.y(), v.z(), 1);
    Transform(v2);
    v.Set(v2.x(), v2.y(), v2.z());
  }
  auto Transform(Vec2f& v) const -> void
  {
    auto v2 = Vec4f(v.x(), v.y(), 1, 1);
    Transform(v2);
    v.Set(v2.x(), v2.y());
  }

  auto TransformDirection(Vec3f& v) const -> void
  {
    auto v2 = Vec4f(v.x(), v.y(), v.z(), 0);
    Transform(v2);
    v.Set(v2.x(), v2.y(), v2.z());
  }

  auto Write(FILE* F = stdout) const -> void;
  auto Write3x3(FILE* F = stdout) const -> void;
  auto Read(FILE* F) -> void;
  auto Read3x3(FILE* F) -> void;

  float data[4][4]{};
};
