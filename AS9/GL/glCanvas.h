#pragma once

#include <cstdlib>

#include "vectors.h"

struct Parser;

struct GLCanvas final {
  GLCanvas() {}
  ~GLCanvas() {}

  static auto initialize(Parser* parser, float refresh, float dt, int integrator_color,
                         int draw_vectors, float acceleration_scale, int motion_blur) -> void;

private:
  static Parser* parser;
  static Vec3f camera_pos;
  static int width;
  static int height;
  static int mouse_button;
  static int mouse_x;
  static int mouse_y;
  static int paused;
  static float refresh;
  static float dt;
  static int integrator_color;
  static int draw_vectors;
  static float acceleration_scale;
  static int motion_blur;

  static auto display() -> void;
  static auto reshape(int w, int h) -> void;
  static auto mouse(int button, int state, int x, int y) -> void;
  static auto motion(int x, int y) -> void;
  static auto keyboard(unsigned char key, int x, int y) -> void;
  static auto idle(int value) -> void;
  static auto step() -> void;
  static auto restart() -> void;
};
