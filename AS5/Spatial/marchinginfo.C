#include "marchinginfo.h"

#include <math.h>

static const float MARCH_INF = 1.0e30f;  // 射线步进的最大距离

MarchingInfo::MarchingInfo()
    : tmin(0), t_exit(MARCH_INF), valid(false),
      i(0), j(0), k(0),
      t_next_x(MARCH_INF), t_next_y(MARCH_INF), t_next_z(MARCH_INF),
      d_tx(MARCH_INF), d_ty(MARCH_INF), d_tz(MARCH_INF),
      sign_x(0), sign_y(0), sign_z(0),
      normal(Vec3f(0, 0, 0)) {}


// 选择最小的 t_next_*，前进到相邻体素并更新进入面的法线
void MarchingInfo::nextCell() {
  if (t_next_x < t_next_y) {
    if (t_next_x < t_next_z) {
      i += sign_x;
      tmin = t_next_x;
      t_next_x += d_tx;
      normal = Vec3f(-(float)sign_x, 0.0f, 0.0f);
    } else {
      k += sign_z;
      tmin = t_next_z;
      t_next_z += d_tz;
      normal = Vec3f(0.0f, 0.0f, -(float)sign_z);
    }
  } else {
    if (t_next_y < t_next_z) {
      j += sign_y;
      tmin = t_next_y;
      t_next_y += d_ty;
      normal = Vec3f(0.0f, -(float)sign_y, 0.0f);
    } else {
      k += sign_z;
      tmin = t_next_z;
      t_next_z += d_tz;
      normal = Vec3f(0.0f, 0.0f, -(float)sign_z);
    }
  }
}

float MarchingInfo::getTMin() const { return tmin; }
int MarchingInfo::getI() const { return i; }
int MarchingInfo::getJ() const { return j; }
int MarchingInfo::getK() const { return k; }
float MarchingInfo::getTNextX() const { return t_next_x; }
float MarchingInfo::getTNextY() const { return t_next_y; }
float MarchingInfo::getTNextZ() const { return t_next_z; }
float MarchingInfo::getDTx() const { return d_tx; }
float MarchingInfo::getDTy() const { return d_ty; }
float MarchingInfo::getDTz() const { return d_tz; }
int MarchingInfo::getSignX() const { return sign_x; }
int MarchingInfo::getSignY() const { return sign_y; }
int MarchingInfo::getSignZ() const { return sign_z; }
Vec3f MarchingInfo::getNormal() const { return normal; }
bool MarchingInfo::isValid() const { return valid; }
float MarchingInfo::getTExit() const { return t_exit; }

void MarchingInfo::setTMin(float t) { tmin = t; }
void MarchingInfo::setI(int v) { i = v; }
void MarchingInfo::setJ(int v) { j = v; }
void MarchingInfo::setK(int v) { k = v; }
void MarchingInfo::setTNextX(float t) { t_next_x = t; }
void MarchingInfo::setTNextY(float t) { t_next_y = t; }
void MarchingInfo::setTNextZ(float t) { t_next_z = t; }
void MarchingInfo::setDTx(float d) { d_tx = d; }
void MarchingInfo::setDTy(float d) { d_ty = d; }
void MarchingInfo::setDTz(float d) { d_tz = d; }
void MarchingInfo::setSignX(int s) { sign_x = s; }
void MarchingInfo::setSignY(int s) { sign_y = s; }
void MarchingInfo::setSignZ(int s) { sign_z = s; }
void MarchingInfo::setNormal(const Vec3f &n) { normal = n; }
void MarchingInfo::setTExit(float t) { t_exit = t; }
void MarchingInfo::setValid(bool v) { valid = v; }
