#pragma once

#include "vectors.h"
#include "random.h"

struct Particle;

struct Generator {
  Generator();
  virtual ~Generator();

  auto SetColors(Vec3f color, Vec3f dead_color, float color_randomness) -> void;
  auto SetLifespan(float lifespan, float lifespan_randomness, int desired_num_particles) -> void;
  auto SetMass(float mass, float mass_randomness) -> void;

  virtual auto numNewParticles(float current_time, float dt) const -> int;
  virtual auto Generate(float current_time, int i) -> Particle* = 0;
  virtual auto Paint() const -> void {}

  virtual auto Restart() -> void;

  auto jitterColor() -> Vec3f;
  auto jitterScalar(float base, float randomness) -> float;

  Vec3f color{};
  Vec3f dead_color{};
  float color_randomness{};
  float mass{};
  float mass_randomness{};
  float lifespan{};
  float lifespan_randomness{};
  int desired_num_particles{};
  Random* rng{};
};

#include "hose_generator.h"
#include "ring_generator.h"
