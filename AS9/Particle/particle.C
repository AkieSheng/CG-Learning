#include "particle.h"
#include "gl_headers.h"

auto Particle::increaseAge(float a) -> void
{
  auto t = 1.0f;
  if (timeToDeath > 0)
  {
    t = a / timeToDeath;
  }
  if (t > 1.0f)
  {
    t = 1.0f;
  }
  color += t * (dead_color - color);
  timeToDeath -= a;
}

auto Particle::Paint(float dt, int integrator_color, int draw_vectors, int motion_blur) const -> void
{
  if (integrator_color == 0)
  {
    ::glColor3f(color.x(), color.y(), color.z());
  }

  if (motion_blur == 0)
  {
    ::glPointSize(3);
    ::glBegin(GL_POINTS);
    ::glVertex3f(position.x(), position.y(), position.z());
    ::glEnd();
  }

  if (draw_vectors == 1)
  {
    ::glLineWidth(1);
    ::glBegin(GL_LINES);
    auto a = position;
    auto b = position + dt * velocity;
    ::glVertex3f(a.x(), a.y(), a.z());
    ::glVertex3f(b.x(), b.y(), b.z());
    ::glEnd();
  }

  if (motion_blur == 1)
  {
    ::glLineWidth(1);
    ::glBegin(GL_LINES);
    auto a = position;
    auto b = last_position;
    ::glVertex3f(a.x(), a.y(), a.z());
    ::glVertex3f(b.x(), b.y(), b.z());
    ::glEnd();
  }
}
