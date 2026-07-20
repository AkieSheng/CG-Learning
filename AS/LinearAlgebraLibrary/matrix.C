#include "matrix.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

auto det2x2(float a, float b, float c, float d) -> float
{
  return a * d - b * c;
}

auto det3x3(
    float a1, float a2, float a3,
    float b1, float b2, float b3,
    float c1, float c2, float c3) -> float
{
  return a1 * det2x2(b2, b3, c2, c3) - b1 * det2x2(a2, a3, c2, c3) +
         c1 * det2x2(a2, a3, b2, b3);
}

auto det4x4(
    float a1, float a2, float a3, float a4,
    float b1, float b2, float b3, float b4,
    float c1, float c2, float c3, float c4,
    float d1, float d2, float d3, float d4) -> float
{
  return a1 * det3x3(b2, b3, b4, c2, c3, c4, d2, d3, d4) -
         b1 * det3x3(a2, a3, a4, c2, c3, c4, d2, d3, d4) +
         c1 * det3x3(a2, a3, a4, b2, b3, b4, d2, d3, d4) -
         d1 * det3x3(a2, a3, a4, b2, b3, b4, c2, c3, c4);
}

}  // namespace

Matrix::Matrix(Matrix const& m)
{
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      data[y][x] = m.data[y][x];
    }
  }
}

Matrix::Matrix(float const* m)
{
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      data[y][x] = m[4 * y + x];
    }
  }
}

auto Matrix::SetToIdentity() -> void
{
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      data[y][x] = (x == y);
    }
  }
}

auto Matrix::Clear() -> void
{
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      data[y][x] = 0;
    }
  }
}

auto Matrix::Transpose(Matrix& m) const -> void
{
  auto tmp = Matrix(*this);
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      m.data[y][x] = tmp.data[x][y];
    }
  }
}

auto Matrix::Inverse(Matrix& m, float epsilon) const -> int
{
  m = *this;

  auto a1 = m.data[0][0];
  auto b1 = m.data[0][1];
  auto c1 = m.data[0][2];
  auto d1 = m.data[0][3];
  auto a2 = m.data[1][0];
  auto b2 = m.data[1][1];
  auto c2 = m.data[1][2];
  auto d2 = m.data[1][3];
  auto a3 = m.data[2][0];
  auto b3 = m.data[2][1];
  auto c3 = m.data[2][2];
  auto d3 = m.data[2][3];
  auto a4 = m.data[3][0];
  auto b4 = m.data[3][1];
  auto c4 = m.data[3][2];
  auto d4 = m.data[3][3];

  auto det = det4x4(a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4);

  if (::fabs(det) < epsilon) {
    ::printf("Matrix::Inverse --- singular matrix, can't invert!\n");
    assert(0);
    return 0;
  }

  m.data[0][0] = det3x3(b2, b3, b4, c2, c3, c4, d2, d3, d4);
  m.data[1][0] = -det3x3(a2, a3, a4, c2, c3, c4, d2, d3, d4);
  m.data[2][0] = det3x3(a2, a3, a4, b2, b3, b4, d2, d3, d4);
  m.data[3][0] = -det3x3(a2, a3, a4, b2, b3, b4, c2, c3, c4);

  m.data[0][1] = -det3x3(b1, b3, b4, c1, c3, c4, d1, d3, d4);
  m.data[1][1] = det3x3(a1, a3, a4, c1, c3, c4, d1, d3, d4);
  m.data[2][1] = -det3x3(a1, a3, a4, b1, b3, b4, d1, d3, d4);
  m.data[3][1] = det3x3(a1, a3, a4, b1, b3, b4, c1, c3, c4);

  m.data[0][2] = det3x3(b1, b2, b4, c1, c2, c4, d1, d2, d4);
  m.data[1][2] = -det3x3(a1, a2, a4, c1, c2, c4, d1, d2, d4);
  m.data[2][2] = det3x3(a1, a2, a4, b1, b2, b4, d1, d2, d4);
  m.data[3][2] = -det3x3(a1, a2, a4, b1, b2, b4, c1, c2, c4);

  m.data[0][3] = -det3x3(b1, b2, b3, c1, c2, c3, d1, d2, d3);
  m.data[1][3] = det3x3(a1, a2, a3, c1, c2, c3, d1, d2, d3);
  m.data[2][3] = -det3x3(a1, a2, a3, b1, b2, b3, d1, d2, d3);
  m.data[3][3] = det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);

  m *= 1 / det;
  return 1;
}

auto Matrix::operator = (Matrix const& m) -> Matrix&
{
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      data[y][x] = m.data[y][x];
    }
  }
  return (*this);
}

auto Matrix::operator == (Matrix const& m) const -> int
{
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      if (this->data[y][x] != m.data[y][x])
        return 0;
    }
  }
  return 1;
}

auto operator + (Matrix const& m1, Matrix const& m2) -> Matrix
{
  auto answer = Matrix{};
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      answer.data[y][x] = m1.data[y][x] + m2.data[y][x];
    }
  }
  return answer;
}

auto operator - (Matrix const& m1, Matrix const& m2) -> Matrix
{
  auto answer = Matrix{};
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      answer.data[y][x] = m1.data[y][x] - m2.data[y][x];
    }
  }
  return answer;
}

auto operator * (Matrix const& m1, Matrix const& m2) -> Matrix
{
  auto answer = Matrix{};
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      for (auto i = 0; i < 4; i++) {
        answer.data[y][x] += m1.data[y][i] * m2.data[i][x];
      }
    }
  }
  return answer;
}

auto operator * (Matrix const& m, float f) -> Matrix
{
  auto answer = Matrix{};
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      answer.data[y][x] = m.data[y][x] * f;
    }
  }
  return answer;
}

auto Matrix::MakeTranslation(Vec3f const& v) -> Matrix
{
  auto t = Matrix{};
  t.SetToIdentity();
  t.data[0][3] = v.x();
  t.data[1][3] = v.y();
  t.data[2][3] = v.z();
  return t;
}

auto Matrix::MakeScale(Vec3f const& v) -> Matrix
{
  auto s = Matrix{};
  s.SetToIdentity();
  s.data[0][0] = v.x();
  s.data[1][1] = v.y();
  s.data[2][2] = v.z();
  s.data[3][3] = 1;
  return s;
}

auto Matrix::MakeXRotation(float theta) -> Matrix
{
  auto rx = Matrix{};
  rx.SetToIdentity();
  rx.data[1][1] = float(::cos(float(theta)));
  rx.data[1][2] = -float(::sin(float(theta)));
  rx.data[2][1] = float(::sin(float(theta)));
  rx.data[2][2] = float(::cos(float(theta)));
  return rx;
}

auto Matrix::MakeYRotation(float theta) -> Matrix
{
  auto ry = Matrix{};
  ry.SetToIdentity();
  ry.data[0][0] = float(::cos(float(theta)));
  ry.data[0][2] = float(::sin(float(theta)));
  ry.data[2][0] = -float(::sin(float(theta)));
  ry.data[2][2] = float(::cos(float(theta)));
  return ry;
}

auto Matrix::MakeZRotation(float theta) -> Matrix
{
  auto rz = Matrix{};
  rz.SetToIdentity();
  rz.data[0][0] = float(::cos(float(theta)));
  rz.data[0][1] = -float(::sin(float(theta)));
  rz.data[1][0] = float(::sin(float(theta)));
  rz.data[1][1] = float(::cos(float(theta)));
  return rz;
}

auto Matrix::MakeAxisRotation(Vec3f const& v, float theta) -> Matrix
{
  auto r = Matrix{};
  r.SetToIdentity();

  auto x = v.x();
  auto y = v.y();
  auto z = v.z();

  auto c = ::cosf(theta);
  auto s = ::sinf(theta);
  auto xx = x * x;
  auto xy = x * y;
  auto xz = x * z;
  auto yy = y * y;
  auto yz = y * z;
  auto zz = z * z;

  r.Set(0, 0, (1 - c) * xx + c);
  r.Set(0, 1, (1 - c) * xy + z * s);
  r.Set(0, 2, (1 - c) * xz - y * s);
  r.Set(0, 3, 0);

  r.Set(1, 0, (1 - c) * xy - z * s);
  r.Set(1, 1, (1 - c) * yy + c);
  r.Set(1, 2, (1 - c) * yz + x * s);
  r.Set(1, 3, 0);

  r.Set(2, 0, (1 - c) * xz + y * s);
  r.Set(2, 1, (1 - c) * yz - x * s);
  r.Set(2, 2, (1 - c) * zz + c);
  r.Set(2, 3, 0);

  r.Set(3, 0, 0);
  r.Set(3, 1, 0);
  r.Set(3, 2, 0);
  r.Set(3, 3, 1);

  return r;
}

auto Matrix::Transform(Vec4f& v) const -> void
{
  auto answer = Vec4f{};
  for (auto y = 0; y < 4; y++) {
    answer.data[y] = 0;
    for (auto i = 0; i < 4; i++) {
      answer.data[y] += data[y][i] * v[i];
    }
  }
  v = answer;
}

auto Matrix::Write(FILE* F) const -> void
{
  assert(F != nullptr);
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      auto tmp = data[y][x];
      if (::fabs(tmp) < 0.00001)
        tmp = 0;
      ::fprintf(F, "%12.6f ", tmp);
    }
    ::fprintf(F, "\n");
  }
}

auto Matrix::Write3x3(FILE* F) const -> void
{
  assert(F != nullptr);
  for (auto y = 0; y < 4; y++) {
    if (y == 2)
      continue;
    for (auto x = 0; x < 4; x++) {
      if (x == 2)
        continue;
      auto tmp = data[y][x];
      if (::fabs(tmp) < 0.00001)
        tmp = 0;
      ::fprintf(F, "%12.6f ", tmp);
    }
    ::fprintf(F, "\n");
  }
}

auto Matrix::Read(FILE* F) -> void
{
  assert(F != nullptr);
  for (auto y = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++) {
      auto scanned = ::fscanf(F, "%f", &data[y][x]);
      assert(scanned == 1);
    }
  }
}

auto Matrix::Read3x3(FILE* F) -> void
{
  assert(F != nullptr);
  Clear();
  for (auto y = 0; y < 4; y++) {
    if (y == 2)
      continue;
    for (auto x = 0; x < 4; x++) {
      if (x == 2)
        continue;
      auto scanned = ::fscanf(F, "%f", &data[y][x]);
      assert(scanned == 1);
    }
  }
}
