#pragma once

#include <cassert>

#include "particle.h"

struct ParticleSet final {
  ParticleSet(int n)
  {
    size = n;
    num_particles = 0;
    particles = new Particle*[size];
    for (auto i = 0; i < size; i++) {
      particles[i] = nullptr;
    }
  }
  ~ParticleSet()
  {
    for (auto i = 0; i < num_particles; i++) {
      delete particles[i];
    }
    delete[] particles;
  }

  auto getNumParticles() const -> int { return num_particles; }
  auto Get(int i) const -> Particle*
  {
    assert(i >= 0 && i < num_particles);
    assert(particles[i] != nullptr);
    return particles[i];
  }

  auto Add(Particle* p) -> void
  {
    assert(p != nullptr);
    if (num_particles == size)
    {
      auto** new_particles = new Particle*[size * 2];
      for (auto i = 0; i < size; i++) {
        new_particles[i] = particles[i];
        new_particles[i + size] = nullptr;
      }
      delete[] particles;
      particles = new_particles;
      size *= 2;
    }
    assert(num_particles < size);
    particles[num_particles] = p;
    num_particles++;
  }

  auto RemoveDead() -> void
  {
    auto i = 0;
    while (1)
    {
      if (i == num_particles)
      {
        break;
      }
      assert(particles[i] != nullptr);
      if (particles[i]->isDead()) {
        delete particles[i];
        num_particles--;
        particles[i] = particles[num_particles];
        particles[num_particles] = nullptr;
      } else {
        i++;
      }
    }
    for (i = 0; i < size; i++) {
      if (i < num_particles)
      {
        assert(particles[i] != nullptr);
        assert(!particles[i]->isDead());
      } else {
        assert(particles[i] == nullptr);
      }
    }
  }

  ParticleSet()
  { assert(0); }

  int num_particles{};
  int size{};
  Particle** particles{};
};
