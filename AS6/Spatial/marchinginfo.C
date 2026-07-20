#include "marchinginfo.h"
#include "raytracing_stats.h"

#include <cmath>

static const float MARCH_INF = 1.0e30f;

MarchingInfo::MarchingInfo()
    : tmin(0), t_exit(MARCH_INF), valid(false),
      i(0), j(0), k(0),
      t_next_x(MARCH_INF), t_next_y(MARCH_INF), t_next_z(MARCH_INF),
      d_tx(MARCH_INF), d_ty(MARCH_INF), d_tz(MARCH_INF),
      sign_x(0), sign_y(0), sign_z(0),
      normal(Vec3f(0, 0, 0)) {}



auto MarchingInfo::nextCell() -> void {
  RayTracingStats::IncrementNumGridCellsTraversed();
  if (t_next_x < t_next_y) {
    if (t_next_x < t_next_z) {
      i += sign_x;
      tmin = t_next_x;
      t_next_x += d_tx;
      normal = Vec3f(-static_cast<float>(sign_x), 0.0f, 0.0f);
    } else {
      k += sign_z;
      tmin = t_next_z;
      t_next_z += d_tz;
      normal = Vec3f(0.0f, 0.0f, -static_cast<float>(sign_z));
    }
  } else {
    if (t_next_y < t_next_z) {
      j += sign_y;
      tmin = t_next_y;
      t_next_y += d_ty;
      normal = Vec3f(0.0f, -static_cast<float>(sign_y), 0.0f);
    } else {
      k += sign_z;
      tmin = t_next_z;
      t_next_z += d_tz;
      normal = Vec3f(0.0f, 0.0f, -static_cast<float>(sign_z));
    }
  }
}

auto MarchingInfo::getTMin()const -> float { return tmin; }
auto MarchingInfo::getI()const -> int { return i; }
auto MarchingInfo::getJ()const -> int { return j; }
auto MarchingInfo::getK()const -> int { return k; }
auto MarchingInfo::getTNextX()const -> float { return t_next_x; }
auto MarchingInfo::getTNextY()const -> float { return t_next_y; }
auto MarchingInfo::getTNextZ()const -> float { return t_next_z; }
auto MarchingInfo::getDTx()const -> float { return d_tx; }
auto MarchingInfo::getDTy()const -> float { return d_ty; }
auto MarchingInfo::getDTz()const -> float { return d_tz; }
auto MarchingInfo::getSignX()const -> int { return sign_x; }
auto MarchingInfo::getSignY()const -> int { return sign_y; }
auto MarchingInfo::getSignZ()const -> int { return sign_z; }
auto MarchingInfo::getNormal()const -> Vec3f { return normal; }
auto MarchingInfo::isValid()const -> bool { return valid; }
auto MarchingInfo::getTExit()const -> float { return t_exit; }

auto MarchingInfo::setTMin(float t) -> void { tmin = t; }
auto MarchingInfo::setI(int v) -> void { i = v; }
auto MarchingInfo::setJ(int v) -> void { j = v; }
auto MarchingInfo::setK(int v) -> void { k = v; }
auto MarchingInfo::setTNextX(float t) -> void { t_next_x = t; }
auto MarchingInfo::setTNextY(float t) -> void { t_next_y = t; }
auto MarchingInfo::setTNextZ(float t) -> void { t_next_z = t; }
auto MarchingInfo::setDTx(float d) -> void { d_tx = d; }
auto MarchingInfo::setDTy(float d) -> void { d_ty = d; }
auto MarchingInfo::setDTz(float d) -> void { d_tz = d; }
auto MarchingInfo::setSignX(int s) -> void { sign_x = s; }
auto MarchingInfo::setSignY(int s) -> void { sign_y = s; }
auto MarchingInfo::setSignZ(int s) -> void { sign_z = s; }
auto MarchingInfo::setNormal(Vec3f const&n) -> void { normal = n; }
auto MarchingInfo::setTExit(float t) -> void { t_exit = t; }
auto MarchingInfo::setValid(bool v) -> void { valid = v; }
