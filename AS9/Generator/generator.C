#include "generator.h"

#include "particle.h"

#include <cassert>
#include <cmath>

Generator::Generator() {
  color = Vec3f(1, 1, 1);
  dead_color = Vec3f(1, 1, 1);
  color_randomness = 0;
  mass = 1;
  mass_randomness = 0;
  lifespan = 10;
  lifespan_randomness = 0;
  desired_num_particles = 1000;
  rng = new Random(0);
}

Generator::~Generator() {
  delete rng;
}

auto Generator::SetColors(Vec3f c, Vec3f dc, float cr) -> void {
  color = c;
  dead_color = dc;
  color_randomness = cr;
}

auto Generator::SetLifespan(float life, float life_r, int desired) -> void {
  lifespan = life;
  lifespan_randomness = life_r;
  desired_num_particles = desired;
}

auto Generator::SetMass(float m, float m_r) -> void {
  mass = m;
  mass_randomness = m_r;
}

auto Generator::numNewParticles(float /*current_time*/, float dt) const -> int {
  assert(lifespan > 0);
  auto n = static_cast<int>(dt * desired_num_particles / lifespan);
  if (n == 0 && dt > 0 && desired_num_particles > 0) {
    n = 1;
  }
  return n;
}

auto Generator::Restart() -> void {
  delete rng;
  rng = new Random(0);
}

auto Generator::jitterColor() -> Vec3f {
  auto c = color + color_randomness * rng->randomVector();
  auto x = c.x();
  if (x < 0) {
    x = 0;
  }
  if (x > 1) {
    x = 1;
  }
  auto y = c.y();
  if (y < 0) {
    y = 0;
  }
  if (y > 1) {
    y = 1;
  }
  auto z = c.z();
  if (z < 0) {
    z = 0;
  }
  if (z > 1) {
    z = 1;
  }
  return Vec3f(x, y, z);
}

auto Generator::jitterScalar(float base, float randomness) -> float {
  auto s = base * (1.0f + randomness * (rng->next() * 2.0f - 1.0f));
  if (s < 1e-6f) {
    s = 1e-6f;
  }
  return s;
}
