#ifndef _MARCHING_INFO_H_
#define _MARCHING_INFO_H_

#include "vectors.h"

// 3D DDA 射线步进状态
class MarchingInfo {
public:
  MarchingInfo();

  void nextCell();  // 步进到下一个体素

  float getTMin() const;
  int getI() const;
  int getJ() const;
  int getK() const;
  float getTNextX() const;
  float getTNextY() const;
  float getTNextZ() const;
  float getDTx() const;
  float getDTy() const;
  float getDTz() const;
  int getSignX() const;
  int getSignY() const;
  int getSignZ() const;
  Vec3f getNormal() const;

  void setTMin(float t);
  void setI(int i);
  void setJ(int j);
  void setK(int k);
  void setTNextX(float t);
  void setTNextY(float t);
  void setTNextZ(float t);
  void setDTx(float d);
  void setDTy(float d);
  void setDTz(float d);
  void setSignX(int s);
  void setSignY(int s);
  void setSignZ(int s);
  void setNormal(const Vec3f &n);
  void setTExit(float t);
  void setValid(bool v);

  bool isValid() const;
  float getTExit() const;

private:
  float tmin;  // 进入参数
  float t_exit;  // 离开参数
  bool valid;  // 是否有效
  int i, j, k;  // 体素索引
  float t_next_x, t_next_y, t_next_z;  // 步进参数
  float d_tx, d_ty, d_tz;  // 步进方向
  int sign_x, sign_y, sign_z;  // 步进方向符号
  Vec3f normal;  // 进入面法线
};

#endif
