#include "gl_headers.h"
#include "ring_generator.h"
#include "particle.h"
#include <assert.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const float RING_GROUND_Y = -4.0f; // 地面高度

RingGenerator::RingGenerator(float position_randomness,
                             Vec3f velocity, float velocity_randomness) {
  this->position_randomness = position_randomness;
  this->velocity = velocity;
  this->velocity_randomness = velocity_randomness;
}

// 生成新粒子数
int RingGenerator::numNewParticles(float current_time, float dt) const {
  assert(lifespan > 0);
  int n = (int)(dt * desired_num_particles / lifespan * current_time);  // n = (int)(dt * desired / lifespan * current_time)
  return n;
}

// 生成粒子
Particle* RingGenerator::Generate(float current_time, int /*i*/) {
  assert(rng != NULL);

  // 半径随时间扩张，角度在 [0, 2π) 均匀随机
  float radius = current_time;
  float theta = float(2.0 * M_PI) * rng->next();

  Vec3f p(radius * cosf(theta), RING_GROUND_Y, radius * sinf(theta));
  p = p + position_randomness * rng->randomVector();  // 位置扰动

  Vec3f v = velocity + velocity_randomness * rng->randomVector();  // 速度扰动

  Vec3f c = jitterColor();  // 颜色扰动
  float m = jitterScalar(mass, mass_randomness);  // 质量扰动
  float life = jitterScalar(lifespan, lifespan_randomness);  // 寿命扰动

  return new Particle(p, v, c, dead_color, m, life);  // 生成粒子
}

// 绘制地面多边形
void RingGenerator::Paint() const {
  GLfloat diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
  GLfloat specular[] = { 0, 0, 0, 1 };
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
  const float s = 12.0f;
  glBegin(GL_QUADS);
  glNormal3f(0, 1, 0);
  glVertex3f(-s, RING_GROUND_Y, -s);
  glVertex3f( s, RING_GROUND_Y, -s);
  glVertex3f( s, RING_GROUND_Y,  s);
  glVertex3f(-s, RING_GROUND_Y,  s);
  glEnd();
}
