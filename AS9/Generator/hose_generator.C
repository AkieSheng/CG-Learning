#include "hose_generator.h"
#include <cassert>
#include "particle.h"

HoseGenerator::HoseGenerator(Vec3f position, float position_randomness, Vec3f velocity,
                             float velocity_randomness)
{
  this->position = position;
  this->position_randomness = position_randomness;
  this->velocity = velocity;
  this->velocity_randomness = velocity_randomness;
}

auto HoseGenerator::Generate(float, int) -> Particle*
{
  assert(rng != nullptr);

  auto p = position + position_randomness * rng->randomVector();
  auto v = velocity + velocity_randomness * rng->randomVector();

  auto c = jitterColor();
  auto m = jitterScalar(mass, mass_randomness);
  auto life = jitterScalar(lifespan, lifespan_randomness);

  return new Particle(p, v, c, dead_color, m, life);
}
