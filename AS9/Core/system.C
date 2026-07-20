#include "system.h"
#include "gl_headers.h"
#include "generator.h"
#include "integrator.h"
#include "forcefield.h"
#include "particleSet.h"
#include "particle.h"

System::System(Generator* g, Integrator* i, ForceField* f)
{
  assert(g != nullptr);
  assert(i != nullptr);
  assert(f != nullptr);
  generator = g;
  integrator = i;
  forcefield = f;
  particles = nullptr;
  Restart();
}

System::~System()
{
  delete generator;
  delete integrator;
  delete forcefield;
  delete particles;
}

auto System::Restart() -> void
{
  delete particles;
  particles = new ParticleSet(100);
  generator->Restart();
  current_time = 0;
}

auto System::Update(float dt) -> void
{
  auto num_particles = particles->getNumParticles();
  for (auto i = 0; i < num_particles; i++) {
    integrator->Update(particles->Get(i), forcefield, current_time, dt);
  }

  auto num_new = generator->numNewParticles(current_time, dt);
  for (auto i = 0; i < num_new; i++) {
    auto* p = generator->Generate(current_time, i);
    assert(p != nullptr);
    particles->Add(p);
  }

  particles->RemoveDead();
  current_time += dt;
}

auto System::PaintGeometry() const -> void
{
  generator->Paint();
}

auto System::Paint(float dt, int integrator_color, int draw_vectors, float acceleration_scale,
                   int motion_blur) const -> void {
  if (integrator_color)
  {
    auto c = integrator->getColor();
    ::glColor3f(c.r(), c.g(), c.b());
  }

  auto num_particles = particles->getNumParticles();
  for (auto i = 0; i < num_particles; i++) {
    particles->Get(i)->Paint(dt, integrator_color, draw_vectors, motion_blur);
  }

  if (draw_vectors)
  {
    ::glColor3f(1, 1, 1);
    ::glBegin(GL_LINES);
    for (auto i = 0; i < num_particles; i++) {
      auto* p = particles->Get(i);
      auto a = p->getPosition();
      auto b = forcefield->getAcceleration(a, p->getMass(), current_time);
      b *= acceleration_scale;
      b += a;
      ::glVertex3f(a.x(), a.y(), a.z());
      ::glVertex3f(b.x(), b.y(), b.z());
    }
    ::glEnd();
  }
}
