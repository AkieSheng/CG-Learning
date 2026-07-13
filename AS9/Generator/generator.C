#include "generator.h"
#include "particle.h"
#include <assert.h>
#include <math.h>

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

void Generator::SetColors(Vec3f c, Vec3f dc, float cr) {
  color = c;
  dead_color = dc;
  color_randomness = cr;
}

void Generator::SetLifespan(float life, float life_r, int desired) {
  lifespan = life;
  lifespan_randomness = life_r;
  desired_num_particles = desired;
}

void Generator::SetMass(float m, float m_r) {
  mass = m;
  mass_randomness = m_r;
}

// 稳态粒子数 ≈ desired_num_particles
// 每步生成 dt * desired / lifespan
int Generator::numNewParticles(float /*current_time*/, float dt) const {
  assert(lifespan > 0);
  int n = (int)(dt * desired_num_particles / lifespan);
  if (n == 0 && dt > 0 && desired_num_particles > 0)
    n = 1;
  return n;
}

// 生成随机种子
void Generator::Restart() {
  delete rng;
  rng = new Random(0);
}

// 颜色扰动
Vec3f Generator::jitterColor() {
  Vec3f c = color + color_randomness * rng->randomVector();  // c = color + randomness * randomVector()
  // 钳到 [0,1]
  float x = c.x(); if (x < 0) x = 0; if (x > 1) x = 1;
  float y = c.y(); if (y < 0) y = 0; if (y > 1) y = 1;
  float z = c.z(); if (z < 0) z = 0; if (z > 1) z = 1;
  return Vec3f(x, y, z);
}

// 标量扰动
float Generator::jitterScalar(float base, float randomness) {
  float s = base * (1.0f + randomness * (rng->next() * 2.0f - 1.0f));  // s = base * (1 + randomness * U[-1,1])
  // 钳到 [1e-6, inf]
  if (s < 1e-6f) s = 1e-6f;
  return s;
}
