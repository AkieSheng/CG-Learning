#pragma once

#include <cassert>

#include "vectors.h"

struct Particle final {
  Particle(Vec3f p, Vec3f v, Vec3f _color, Vec3f _dead_color, float m, float lifespan)
  {
    position = p;
    last_position = p;
    velocity = v;
    color = _color;
    dead_color = _dead_color;
    mass = m;
    timeToDeath = lifespan;
  }
  ~Particle()
  { }

  auto getPosition() const -> Vec3f { return position; }
  auto getVelocity() const -> Vec3f { return velocity; }
  auto getColor() const -> Vec3f { return color; }
  auto getMass() const -> float { return mass; }
  auto isDead() const -> int
  {
    if (timeToDeath <= 0)
    {
      return 1;
    }
    return 0;
  }

  auto setPosition(Vec3f p) -> void
  {
    last_position = position;
    position = p;
  }
  auto setVelocity(Vec3f v) -> void { velocity = v; }
  auto setColor(Vec3f c) -> void { color = c; }
  auto setMass(float m) -> void { mass = m; }
  auto increaseAge(float a) -> void;

  auto Paint(float dt, int integrator_color, int draw_vectors, int motion_blur) const -> void;

  Particle()
  { assert(0); }

  Vec3f position{};
  Vec3f last_position{};
  Vec3f velocity{};
  Vec3f color{};
  Vec3f dead_color{};
  float mass{};
  float timeToDeath{};
};
