#pragma once

#include "generator.h"

struct RingGenerator final : Generator {
  RingGenerator(float position_randomness, Vec3f velocity, float velocity_randomness);
  ~RingGenerator() override {}

  auto numNewParticles(float current_time, float dt) const -> int override;
  auto Generate(float current_time, int i) -> Particle* override;
  auto Paint() const -> void override;

  float position_randomness{};
  Vec3f velocity{};
  float velocity_randomness{};
};
