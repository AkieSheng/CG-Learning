#include "euler_integrator.h"
#include "particle.h"
#include "forcefield.h"
#include <assert.h>

// Euler 显式积分
// 用当前点加速度更新速度，用当前速度更新位置
void EulerIntegrator::Update(Particle *particle, ForceField *forcefield,
                             float t, float dt) {
  assert(particle != NULL);
  assert(forcefield != NULL);

  Vec3f p = particle->getPosition();
  Vec3f v = particle->getVelocity();
  float mass = particle->getMass();

  // a(p_n, t)：在当前位置、当前时间采样加速度
  Vec3f a = forcefield->getAcceleration(p, mass, t);

  // p_{n+1} = p_n + v_n * dt
  // v_{n+1} = v_n + a(p_n, t) * dt
  particle->setPosition(p + dt * v);
  particle->setVelocity(v + dt * a);

  // 推进寿命时钟
  particle->increaseAge(dt);
}
