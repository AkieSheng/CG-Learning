#pragma once

#include "vectors.h"
#include "matrix.h"

struct OrbitCamera final
{
  OrbitCamera();

  auto setTarget(Vec3f const& t) -> void;
  auto setDistance(float d) -> void;
  auto setAspect(float aspect) -> void;
  auto setFovY(float degrees) -> void;

  auto rotate(float deltaYaw, float deltaPitch) -> void;
  auto pan(float deltaX, float deltaY) -> void;
  auto zoom(float delta) -> void;

  auto updateMatrices() -> void;

  auto getViewMatrix() const -> Matrix const& { return viewMatrix; }
  auto getProjectionMatrix() const -> Matrix const& { return projectionMatrix; }
  auto getPosition() const -> Vec3f { return position; }
  auto getTarget() const -> Vec3f { return target; }
  auto getDistance() const -> float { return distance; }
  auto getNear() const -> float { return nearZ; }
  auto getFar() const -> float { return farZ; }
  auto getFront() const -> Vec3f;
  auto getUp() const -> Vec3f { return worldUp; }

  auto frameBounds(Vec3f const& bmin, Vec3f const& bmax) -> void;

  Vec3f target{};
  Vec3f worldUp{0.0f, 1.0f, 0.0f};
  float distance{5.0f};
  float yaw{};
  float pitch{20.0f};
  float fovY{45.0f};
  float aspect{1.0f};
  float nearZ{0.01f};
  float farZ{100.0f};

  Vec3f position{};
  Matrix viewMatrix{};
  Matrix projectionMatrix{};

  auto updatePosition() -> void;
};
