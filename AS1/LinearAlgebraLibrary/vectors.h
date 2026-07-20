#pragma once

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cassert>

struct Matrix;

struct Vec2f final
{
  Vec2f() { data[0] = data[1] = 0; }
  Vec2f(Vec2f const& V)
  {
    data[0] = V.data[0];
    data[1] = V.data[1];
  }
  Vec2f(float d0, float d1)
  {
    data[0] = d0;
    data[1] = d1;
  }
  Vec2f(Vec2f const& V1, Vec2f const& V2)
  {
    data[0] = V1.data[0] - V2.data[0];
    data[1] = V1.data[1] - V2.data[1];
  }
  ~Vec2f() {}

  auto Get(float& d0, float& d1) const -> void
  {
    d0 = data[0];
    d1 = data[1];
  }
  auto operator [] (int i) const -> float
  {
    assert((i >= 0) && (i < 2));
    return data[i];
  }
  auto x() const -> float { return data[0]; }
  auto y() const -> float { return data[1]; }
  auto Length() const -> float
  {
    return float(::sqrt(data[0] * data[0] + data[1] * data[1]));
  }

  auto Set(float d0, float d1) -> void
  {
    data[0] = d0;
    data[1] = d1;
  }
  auto Scale(float d0, float d1) -> void
  {
    data[0] *= d0;
    data[1] *= d1;
  }
  auto Divide(float d0, float d1) -> void
  {
    data[0] /= d0;
    data[1] /= d1;
  }
  auto Negate() -> void
  {
    data[0] = -data[0];
    data[1] = -data[1];
  }

  auto operator = (Vec2f const& V) -> Vec2f&
  {
    data[0] = V.data[0];
    data[1] = V.data[1];
    return *this;
  }
  auto operator == (Vec2f const& V) const -> int
  {
    return ((data[0] == V.data[0]) && (data[1] == V.data[1]));
  }
  auto operator != (Vec2f const& V) -> int
  {
    return ((data[0] != V.data[0]) || (data[1] != V.data[1]));
  }
  auto operator += (Vec2f const& V) -> Vec2f&
  {
    data[0] += V.data[0];
    data[1] += V.data[1];
    return *this;
  }
  auto operator -= (Vec2f const& V) -> Vec2f&
  {
    data[0] -= V.data[0];
    data[1] -= V.data[1];
    return *this;
  }
  auto operator *= (float f) -> Vec2f&
  {
    data[0] *= f;
    data[1] *= f;
    return *this;
  }
  auto operator /= (float f) -> Vec2f&
  {
    data[0] /= f;
    data[1] /= f;
    return *this;
  }

  auto Dot2(Vec2f const& V) const -> float
  {
    return data[0] * V.data[0] + data[1] * V.data[1];
  }

  static auto Add(Vec2f& a, Vec2f const& b, Vec2f const& c) -> void
  {
    a.data[0] = b.data[0] + c.data[0];
    a.data[1] = b.data[1] + c.data[1];
  }
  static auto Sub(Vec2f& a, Vec2f const& b, Vec2f const& c) -> void
  {
    a.data[0] = b.data[0] - c.data[0];
    a.data[1] = b.data[1] - c.data[1];
  }
  static auto CopyScale(Vec2f& a, Vec2f const& b, float c) -> void
  {
    a.data[0] = b.data[0] * c;
    a.data[1] = b.data[1] * c;
  }
  static auto AddScale(Vec2f& a, Vec2f const& b, Vec2f const& c, float d) -> void
  {
    a.data[0] = b.data[0] + c.data[0] * d;
    a.data[1] = b.data[1] + c.data[1] * d;
  }
  static auto Average(Vec2f& a, Vec2f const& b, Vec2f const& c) -> void
  {
    a.data[0] = (b.data[0] + c.data[0]) * 0.5f;
    a.data[1] = (b.data[1] + c.data[1]) * 0.5f;
  }
  static auto WeightedSum(Vec2f& a, Vec2f const& b, float c, Vec2f const& d, float e) -> void
  {
    a.data[0] = b.data[0] * c + d.data[0] * e;
    a.data[1] = b.data[1] * c + d.data[1] * e;
  }

  auto Write(FILE* F = stdout) const -> void
  {
    ::fprintf(F, "%f %f\n", data[0], data[1]);
  }

  float data[2]{};
};

struct Vec3f final
{
  Vec3f() { data[0] = data[1] = data[2] = 0; }
  Vec3f(Vec3f const& V)
  {
    data[0] = V.data[0];
    data[1] = V.data[1];
    data[2] = V.data[2];
  }
  Vec3f(float d0, float d1, float d2)
  {
    data[0] = d0;
    data[1] = d1;
    data[2] = d2;
  }
  Vec3f(Vec3f const& V1, Vec3f const& V2)
  {
    data[0] = V1.data[0] - V2.data[0];
    data[1] = V1.data[1] - V2.data[1];
    data[2] = V1.data[2] - V2.data[2];
  }
  ~Vec3f() {}

  auto Get(float& d0, float& d1, float& d2) const -> void
  {
    d0 = data[0];
    d1 = data[1];
    d2 = data[2];
  }
  auto operator [] (int i) const -> float
  {
    assert((i >= 0) && (i < 3));
    return data[i];
  }
  auto x() const -> float { return data[0]; }
  auto y() const -> float { return data[1]; }
  auto z() const -> float { return data[2]; }
  auto r() const -> float { return data[0]; }
  auto g() const -> float { return data[1]; }
  auto b() const -> float { return data[2]; }
  auto Length() const -> float
  {
    return float(::sqrt(data[0] * data[0] + data[1] * data[1] + data[2] * data[2]));
  }

  auto Set(float d0, float d1, float d2) -> void
  {
    data[0] = d0;
    data[1] = d1;
    data[2] = d2;
  }
  auto Scale(float d0, float d1, float d2) -> void
  {
    data[0] *= d0;
    data[1] *= d1;
    data[2] *= d2;
  }
  auto Divide(float d0, float d1, float d2) -> void
  {
    data[0] /= d0;
    data[1] /= d1;
    data[2] /= d2;
  }
  auto Normalize() -> void
  {
    auto l = Length();
    if (l > 0) {
      data[0] /= l;
      data[1] /= l;
      data[2] /= l;
    }
  }
  auto Negate() -> void
  {
    data[0] = -data[0];
    data[1] = -data[1];
    data[2] = -data[2];
  }
  auto Clamp(float low = 0, float high = 1) -> void
  {
    if (data[0] < low)
      data[0] = low;
    if (data[0] > high)
      data[0] = high;
    if (data[1] < low)
      data[1] = low;
    if (data[1] > high)
      data[1] = high;
    if (data[2] < low)
      data[2] = low;
    if (data[2] > high)
      data[2] = high;
  }

  auto operator = (Vec3f const& V) -> Vec3f&
  {
    data[0] = V.data[0];
    data[1] = V.data[1];
    data[2] = V.data[2];
    return *this;
  }
  auto operator == (Vec3f const& V) -> int
  {
    return ((data[0] == V.data[0]) && (data[1] == V.data[1]) && (data[2] == V.data[2]));
  }
  auto operator != (Vec3f const& V) -> int
  {
    return ((data[0] != V.data[0]) || (data[1] != V.data[1]) || (data[2] != V.data[2]));
  }
  auto operator += (Vec3f const& V) -> Vec3f&
  {
    data[0] += V.data[0];
    data[1] += V.data[1];
    data[2] += V.data[2];
    return *this;
  }
  auto operator -= (Vec3f const& V) -> Vec3f&
  {
    data[0] -= V.data[0];
    data[1] -= V.data[1];
    data[2] -= V.data[2];
    return *this;
  }
  auto operator *= (int i) -> Vec3f&
  {
    data[0] = float(data[0] * i);
    data[1] = float(data[1] * i);
    data[2] = float(data[2] * i);
    return *this;
  }
  auto operator *= (float f) -> Vec3f&
  {
    data[0] *= f;
    data[1] *= f;
    data[2] *= f;
    return *this;
  }
  auto operator /= (int i) -> Vec3f&
  {
    data[0] = float(data[0] / i);
    data[1] = float(data[1] / i);
    data[2] = float(data[2] / i);
    return *this;
  }
  auto operator /= (float f) -> Vec3f&
  {
    data[0] /= f;
    data[1] /= f;
    data[2] /= f;
    return *this;
  }

  friend auto operator + (Vec3f const& v1, Vec3f const& v2) -> Vec3f
  {
    Vec3f v3;
    Add(v3, v1, v2);
    return v3;
  }
  friend auto operator - (Vec3f const& v1, Vec3f const& v2) -> Vec3f
  {
    Vec3f v3;
    Sub(v3, v1, v2);
    return v3;
  }
  friend auto operator * (Vec3f const& v1, float f) -> Vec3f
  {
    Vec3f v2;
    CopyScale(v2, v1, f);
    return v2;
  }
  friend auto operator * (float f, Vec3f const& v1) -> Vec3f
  {
    Vec3f v2;
    CopyScale(v2, v1, f);
    return v2;
  }
  friend auto operator * (Vec3f const& v1, Vec3f const& v2) -> Vec3f
  {
    Vec3f v3;
    Mult(v3, v1, v2);
    return v3;
  }

  auto Dot3(Vec3f const& V) const -> float
  {
    return data[0] * V.data[0] + data[1] * V.data[1] + data[2] * V.data[2];
  }

  static auto Add(Vec3f& a, Vec3f const& b, Vec3f const& c) -> void
  {
    a.data[0] = b.data[0] + c.data[0];
    a.data[1] = b.data[1] + c.data[1];
    a.data[2] = b.data[2] + c.data[2];
  }
  static auto Sub(Vec3f& a, Vec3f const& b, Vec3f const& c) -> void
  {
    a.data[0] = b.data[0] - c.data[0];
    a.data[1] = b.data[1] - c.data[1];
    a.data[2] = b.data[2] - c.data[2];
  }
  static auto Mult(Vec3f& a, Vec3f const& b, Vec3f const& c) -> void
  {
    a.data[0] = b.data[0] * c.data[0];
    a.data[1] = b.data[1] * c.data[1];
    a.data[2] = b.data[2] * c.data[2];
  }
  static auto CopyScale(Vec3f& a, Vec3f const& b, float c) -> void
  {
    a.data[0] = b.data[0] * c;
    a.data[1] = b.data[1] * c;
    a.data[2] = b.data[2] * c;
  }
  static auto AddScale(Vec3f& a, Vec3f const& b, Vec3f const& c, float d) -> void
  {
    a.data[0] = b.data[0] + c.data[0] * d;
    a.data[1] = b.data[1] + c.data[1] * d;
    a.data[2] = b.data[2] + c.data[2] * d;
  }
  static auto Average(Vec3f& a, Vec3f const& b, Vec3f const& c) -> void
  {
    a.data[0] = (b.data[0] + c.data[0]) * 0.5f;
    a.data[1] = (b.data[1] + c.data[1]) * 0.5f;
    a.data[2] = (b.data[2] + c.data[2]) * 0.5f;
  }
  static auto WeightedSum(Vec3f& a, Vec3f const& b, float c, Vec3f const& d, float e) -> void
  {
    a.data[0] = b.data[0] * c + d.data[0] * e;
    a.data[1] = b.data[1] * c + d.data[1] * e;
    a.data[2] = b.data[2] * c + d.data[2] * e;
  }
  static auto Cross3(Vec3f& c, Vec3f const& v1, Vec3f const& v2) -> void
  {
    auto x = v1.data[1] * v2.data[2] - v1.data[2] * v2.data[1];
    auto y = v1.data[2] * v2.data[0] - v1.data[0] * v2.data[2];
    auto z = v1.data[0] * v2.data[1] - v1.data[1] * v2.data[0];
    c.data[0] = x;
    c.data[1] = y;
    c.data[2] = z;
  }

  static auto Min(Vec3f& a, Vec3f const& b, Vec3f const& c) -> void
  {
    a.data[0] = (b.data[0] < c.data[0]) ? b.data[0] : c.data[0];
    a.data[1] = (b.data[1] < c.data[1]) ? b.data[1] : c.data[1];
    a.data[2] = (b.data[2] < c.data[2]) ? b.data[2] : c.data[2];
  }
  static auto Max(Vec3f& a, Vec3f const& b, Vec3f const& c) -> void
  {
    a.data[0] = (b.data[0] > c.data[0]) ? b.data[0] : c.data[0];
    a.data[1] = (b.data[1] > c.data[1]) ? b.data[1] : c.data[1];
    a.data[2] = (b.data[2] > c.data[2]) ? b.data[2] : c.data[2];
  }

  auto Write(FILE* F = stdout) const -> void
  {
    ::fprintf(F, "%f %f %f\n", data[0], data[1], data[2]);
  }

  friend struct Matrix;

  float data[3]{};
};

struct Vec4f final
{
  Vec4f() { data[0] = data[1] = data[2] = data[3] = 0; }
  Vec4f(Vec4f const& V)
  {
    data[0] = V.data[0];
    data[1] = V.data[1];
    data[2] = V.data[2];
    data[3] = V.data[3];
  }
  Vec4f(float d0, float d1, float d2, float d3)
  {
    data[0] = d0;
    data[1] = d1;
    data[2] = d2;
    data[3] = d3;
  }
  Vec4f(Vec3f const& V, float w)
  {
    data[0] = V.x();
    data[1] = V.y();
    data[2] = V.z();
    data[3] = w;
  }
  Vec4f(Vec4f const& V1, Vec4f const& V2)
  {
    data[0] = V1.data[0] - V2.data[0];
    data[1] = V1.data[1] - V2.data[1];
    data[2] = V1.data[2] - V2.data[2];
    data[3] = V1.data[3] - V2.data[3];
  }
  ~Vec4f() {}

  auto Get(float& d0, float& d1, float& d2, float& d3) const -> void
  {
    d0 = data[0];
    d1 = data[1];
    d2 = data[2];
    d3 = data[3];
  }
  auto operator [] (int i) const -> float
  {
    assert((i >= 0) && (i < 4));
    return data[i];
  }
  auto x() const -> float { return data[0]; }
  auto y() const -> float { return data[1]; }
  auto z() const -> float { return data[2]; }
  auto w() const -> float { return data[3]; }
  auto r() const -> float { return data[0]; }
  auto g() const -> float { return data[1]; }
  auto b() const -> float { return data[2]; }
  auto a() const -> float { return data[3]; }
  auto Length() const -> float
  {
    return float(::sqrt(
        data[0] * data[0] + data[1] * data[1] + data[2] * data[2] + data[3] * data[3]));
  }

  auto Set(float d0, float d1, float d2, float d3) -> void
  {
    data[0] = d0;
    data[1] = d1;
    data[2] = d2;
    data[3] = d3;
  }
  auto Scale(float d0, float d1, float d2, float d3) -> void
  {
    data[0] *= d0;
    data[1] *= d1;
    data[2] *= d2;
    data[3] *= d3;
  }
  auto Divide(float d0, float d1, float d2, float d3) -> void
  {
    data[0] /= d0;
    data[1] /= d1;
    data[2] /= d2;
    data[3] /= d3;
  }
  auto Negate() -> void
  {
    data[0] = -data[0];
    data[1] = -data[1];
    data[2] = -data[2];
    data[3] = -data[3];
  }
  auto Normalize() -> void
  {
    auto l = Length();
    if (l > 0) {
      data[0] /= l;
      data[1] /= l;
      data[2] /= l;
    }
  }
  auto DivideByW() -> void
  {
    if (data[3] != 0) {
      data[0] /= data[3];
      data[1] /= data[3];
      data[2] /= data[3];
    } else {
      data[0] = data[1] = data[2] = 0;
    }
    data[3] = 1;
  }

  auto operator = (Vec4f const& V) -> Vec4f&
  {
    data[0] = V.data[0];
    data[1] = V.data[1];
    data[2] = V.data[2];
    data[3] = V.data[3];
    return *this;
  }
  auto operator == (Vec4f const& V) const -> int
  {
    return ((data[0] == V.data[0]) && (data[1] == V.data[1]) && (data[2] == V.data[2]) &&
            (data[3] == V.data[3]));
  }
  auto operator != (Vec4f const& V) const -> int
  {
    return ((data[0] != V.data[0]) || (data[1] != V.data[1]) || (data[2] != V.data[2]) ||
            (data[3] != V.data[3]));
  }
  auto operator += (Vec4f const& V) -> Vec4f&
  {
    data[0] += V.data[0];
    data[1] += V.data[1];
    data[2] += V.data[2];
    data[3] += V.data[3];
    return *this;
  }
  auto operator -= (Vec4f const& V) -> Vec4f&
  {
    data[0] -= V.data[0];
    data[1] -= V.data[1];
    data[2] -= V.data[2];
    data[3] -= V.data[3];
    return *this;
  }
  auto operator *= (float f) -> Vec4f&
  {
    data[0] *= f;
    data[1] *= f;
    data[2] *= f;
    data[3] *= f;
    return *this;
  }
  auto operator /= (float f) -> Vec4f&
  {
    data[0] /= f;
    data[1] /= f;
    data[2] /= f;
    data[3] /= f;
    return *this;
  }

  auto Dot2(Vec4f const& V) const -> float
  {
    return data[0] * V.data[0] + data[1] * V.data[1];
  }
  auto Dot3(Vec4f const& V) const -> float
  {
    return data[0] * V.data[0] + data[1] * V.data[1] + data[2] * V.data[2];
  }
  auto Dot4(Vec4f const& V) const -> float
  {
    return data[0] * V.data[0] + data[1] * V.data[1] + data[2] * V.data[2] + data[3] * V.data[3];
  }

  static auto Add(Vec4f& a, Vec4f const& b, Vec4f const& c) -> void
  {
    a.data[0] = b.data[0] + c.data[0];
    a.data[1] = b.data[1] + c.data[1];
    a.data[2] = b.data[2] + c.data[2];
    a.data[3] = b.data[3] + c.data[3];
  }
  static auto Sub(Vec4f& a, Vec4f const& b, Vec4f const& c) -> void
  {
    a.data[0] = b.data[0] - c.data[0];
    a.data[1] = b.data[1] - c.data[1];
    a.data[2] = b.data[2] - c.data[2];
    a.data[3] = b.data[3] - c.data[3];
  }
  static auto CopyScale(Vec4f& a, Vec4f const& b, float c) -> void
  {
    a.data[0] = b.data[0] * c;
    a.data[1] = b.data[1] * c;
    a.data[2] = b.data[2] * c;
    a.data[3] = b.data[3] * c;
  }
  static auto AddScale(Vec4f& a, Vec4f const& b, Vec4f const& c, float d) -> void
  {
    a.data[0] = b.data[0] + c.data[0] * d;
    a.data[1] = b.data[1] + c.data[1] * d;
    a.data[2] = b.data[2] + c.data[2] * d;
    a.data[3] = b.data[3] + c.data[3] * d;
  }
  static auto Average(Vec4f& a, Vec4f const& b, Vec4f const& c) -> void
  {
    a.data[0] = (b.data[0] + c.data[0]) * 0.5f;
    a.data[1] = (b.data[1] + c.data[1]) * 0.5f;
    a.data[2] = (b.data[2] + c.data[2]) * 0.5f;
    a.data[3] = (b.data[3] + c.data[3]) * 0.5f;
  }
  static auto WeightedSum(Vec4f& a, Vec4f const& b, float c, Vec4f const& d, float e) -> void
  {
    a.data[0] = b.data[0] * c + d.data[0] * e;
    a.data[1] = b.data[1] * c + d.data[1] * e;
    a.data[2] = b.data[2] * c + d.data[2] * e;
    a.data[3] = b.data[3] * c + d.data[3] * e;
  }
  static auto Cross3(Vec4f& c, Vec4f const& v1, Vec4f const& v2) -> void
  {
    auto x = v1.data[1] * v2.data[2] - v1.data[2] * v2.data[1];
    auto y = v1.data[2] * v2.data[0] - v1.data[0] * v2.data[2];
    auto z = v1.data[0] * v2.data[1] - v1.data[1] * v2.data[0];
    c.data[0] = x;
    c.data[1] = y;
    c.data[2] = z;
  }

  auto Write(FILE* F = stdout) const -> void
  {
    ::fprintf(F, "%f %f %f %f\n", data[0], data[1], data[2], data[3]);
  }

  friend struct Matrix;

  float data[4]{};
};

inline auto operator << (std::ostream& os, Vec3f const& v) -> std::ostream&
{
  os << "Vec3f <" << v.x() << ", " << v.y() << ", " << v.z() << ">";
  return os;
}

