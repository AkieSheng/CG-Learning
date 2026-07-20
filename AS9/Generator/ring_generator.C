#include "ring_generator.h"
#include <cassert>
#include <cmath>
#include "gl_headers.h"
#include "particle.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float const RING_GROUND_Y = -4.0f;

RingGenerator::RingGenerator(float position_randomness, Vec3f velocity,
                             float velocity_randomness)
{
  this->position_randomness = position_randomness;
  this->velocity = velocity;
  this->velocity_randomness = velocity_randomness;
}

auto RingGenerator::numNewParticles(float current_time, float dt) const -> int
{
  assert(lifespan > 0);
  return static_cast<int>(dt * desired_num_particles / lifespan * current_time);
}

auto RingGenerator::Generate(float current_time, int) -> Particle*
{
  assert(rng != nullptr);

  auto radius = current_time;
  auto theta = static_cast<float>(2.0 * M_PI) * rng->next();

  auto p = Vec3f(radius * ::cosf(theta), RING_GROUND_Y, radius * ::sinf(theta));
  p = p + position_randomness * rng->randomVector();

  auto v = velocity + velocity_randomness * rng->randomVector();

  auto c = jitterColor();
  auto m = jitterScalar(mass, mass_randomness);
  auto life = jitterScalar(lifespan, lifespan_randomness);

  return new Particle(p, v, c, dead_color, m, life);
}

auto RingGenerator::Paint() const -> void
{
  GLfloat diffuse[] = {0.4f, 0.4f, 0.4f, 1.0f};
  GLfloat specular[] = {0, 0, 0, 1};
  ::glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffuse);
  ::glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
  auto s = 12.0f;
  ::glBegin(GL_QUADS);
  ::glNormal3f(0, 1, 0);
  ::glVertex3f(-s, RING_GROUND_Y, -s);
  ::glVertex3f(s, RING_GROUND_Y, -s);
  ::glVertex3f(s, RING_GROUND_Y, s);
  ::glVertex3f(-s, RING_GROUND_Y, s);
  ::glEnd();
}
