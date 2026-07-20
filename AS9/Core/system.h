#pragma once

#include <cassert>

#include "vectors.h"

struct Particle;
struct ParticleSet;
struct Generator;
struct Integrator;
struct ForceField;

struct System {
  System(Generator* g, Integrator* i, ForceField* f);
  ~System();

  auto Restart() -> void;
  auto Update(float dt) -> void;

  auto Paint(float dt, int integrator_color, int draw_vectors, float acceleration_scale,
             int motion_blur) const -> void;
  auto PaintGeometry() const -> void;

  System()
  { assert(0); }

  ParticleSet* particles{};
  Generator* generator{};
  Integrator* integrator{};
  ForceField* forcefield{};
  float current_time{};
};
