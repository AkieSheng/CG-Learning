#pragma once

#include "vectors.h"

struct MarchingInfo final {
  MarchingInfo();

  auto nextCell() -> void;

  auto getTMin() const -> float;
  auto getI() const -> int;
  auto getJ() const -> int;
  auto getK() const -> int;
  auto getTNextX() const -> float;
  auto getTNextY() const -> float;
  auto getTNextZ() const -> float;
  auto getDTx() const -> float;
  auto getDTy() const -> float;
  auto getDTz() const -> float;
  auto getSignX() const -> int;
  auto getSignY() const -> int;
  auto getSignZ() const -> int;
  auto getNormal() const -> Vec3f;

  auto setTMin(float t) -> void;
  auto setI(int i) -> void;
  auto setJ(int j) -> void;
  auto setK(int k) -> void;
  auto setTNextX(float t) -> void;
  auto setTNextY(float t) -> void;
  auto setTNextZ(float t) -> void;
  auto setDTx(float d) -> void;
  auto setDTy(float d) -> void;
  auto setDTz(float d) -> void;
  auto setSignX(int s) -> void;
  auto setSignY(int s) -> void;
  auto setSignZ(int s) -> void;
  auto setNormal(Vec3f const& n) -> void;
  auto setTExit(float t) -> void;
  auto setValid(bool v) -> void;

  auto isValid() const -> bool;
  auto getTExit() const -> float;

  float tmin{};
  float t_exit{};
  bool valid{};
  int i{};
  int j{};
  int k{};
  float t_next_x{};
  float t_next_y{};
  float t_next_z{};
  float d_tx{};
  float d_ty{};
  float d_tz{};
  int sign_x{};
  int sign_y{};
  int sign_z{};
  Vec3f normal{};
};
