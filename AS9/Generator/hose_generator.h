#pragma once

#include "generator.h"

struct HoseGenerator final : Generator {
  HoseGenerator(Vec3f position, float position_randomness, Vec3f velocity,
                float velocity_randomness);
  ~HoseGenerator() override {}

  auto Generate(float current_time, int i) -> Particle* override;

private:
  Vec3f position{};
  float position_randomness{};
  Vec3f velocity{};
  float velocity_randomness{};
};
